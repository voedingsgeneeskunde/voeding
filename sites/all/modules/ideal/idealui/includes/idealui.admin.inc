<?php

/**
 * @file
 * The iDEAL administration interface.
 */

/**
 * List all transactions.
 *
 * @return string
 */
function idealui_transaction_list() {
  global $pager_page_array, $pager_total, $pager_total_items;

  $page = isset($_GET['page']) ? $_GET['page'] : '';

  // Build the pager. We use Drupal's built-in paging system, but don't use any
  // of the API functions, because they are too SQL-dependent.
  $query = new EntityFieldQuery();
  $count = $query->entityCondition('entity_type', 'ideal_transaction')
    ->count()
    ->execute();
  $pager_page_array = explode(',', $page);
  $pager_total_items[0] = $count;
  $pager_total[0] = ceil($count / 20);
  $pager = theme('pager');

  // Build the table.
  $header = array(
    array(
      'data' => t('Last update'),
      'field' => 'updated',
      'sort' => 'desc',
    ),
    array(
      'data' => t('Amount'),
      'field' => 'amount',
    ),
    array(
      'data' => t('Description'),
      'field' => 'description',
    ),
    array(
      'data' => t('Name'),
      'field' => 'name',
    ),
    array(
      'data' => t('City'),
      'field' => 'city',
    ),
    array(
      'data' => t('Account number'),
      'field' => 'account_number',
    ),
    t('Status'),
  );
  $sort_field = 'updated';
  $order = 'desc';
  if (isset($_GET['order'])) {
    foreach ($header as $cell) {
      if ($cell['data'] == $_GET['order']) {
        $sort_field = $cell['field'];
        $order = isset($_GET['sort']) ? $_GET['sort'] : $cell['sort'];
      }
    }
  }
  $rows = array();
  $query = new EntityFieldQuery();
  $entities = $query->entityCondition('entity_type', 'ideal_transaction')
    ->propertyOrderBy($sort_field, $order)
    ->range($page * 20, 20)
    ->execute();
  if ($entities) {
    $transactions = entity_load('ideal_transaction', array_keys($entities['ideal_transaction']));
    foreach ($transactions as $transaction) {
      $unknown = $transaction->status == IDEAL_STATUS_UNKNOWN;
      $rows[] = array(
        'data' => array(
          format_date($transaction->updated, 'small'),
          '€' . ideal_cents_to_human_readable($transaction->amount),
          $transaction->description,
          $transaction->name,
          $transaction->city,
          $transaction->account_number,
          $unknown ? theme('idealui_status_unknown') : ideal_status_title($transaction->status),
        ),
        'class' => $unknown ? array('error') : NULL,
      );
    }
  }
  else {
    $rows[] = array(
      array(
        'data' => t('There are no transactions yet.'),
        'colspan' => 7,
      ),
    );
  }

  return $pager . theme('table', array(
    'header' => $header,
    'rows' => $rows,
  )) . $pager;
}

/**
 * Implements theme function: display IDEAL_STATUS_UNKNOWN in the transaction overview.
 */
function theme_idealui_status_unknown(array $variables) {
  return ideal_status_title(IDEAL_STATUS_UNKNOWN) . '<div class ="description">' . t("Check status with your bank.") . '</div>';
}

/**
 * The merchant overview page.
 *
 * @return string
 */
function idealui_merchant_list() {
  $rows = array();
  foreach (entity_load('ideal_merchant') as $merchant) {
    $rows[] = array(
      $merchant->title,
      $merchant->merchant_id,
      $merchant->sub_id,
      $merchant->acquirer->title,
      $merchant->imid != variable_get('ideal_default_imid', 1) ? l(t('make default'), 'admin/config/services/ideal/merchant/default/' . $merchant->imid) : t('Default merchant'),
      idealui_access_merchant($merchant, 'edit') ? l(t('edit'), 'admin/config/services/ideal/merchant/edit/' . $merchant->imid) : NULL,
      idealui_access_merchant($merchant, 'delete') ? l(t('delete'), 'admin/config/services/ideal/merchant/delete/' . $merchant->imid) : NULL,
    );
  }

  $header = array(
    t('Title'),
    t('Merchant ID'),
    t('Sub ID'),
    t('Acquirer'),
    array(
      'data' => t('Operations'),
      'colspan' => 3,
    ),
  );

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
  ));
}

/**
 * Implements form build callback: the merchant add/edit form.
 *
 * @param $merchant mixed
 *   Either an iDEALMerchant object to edit it, or NULL to display an empty
 *   form to add a new merchant.
 */
function idealui_form_merchant(array $form, array &$form_state, iDEALMerchant $merchant = NULL) {
  $form['imid'] = array(
    '#type' => 'value',
    '#value' => $merchant ? $merchant->imid : 0,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $merchant ? $merchant->title : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $options = array('<' . t('none') . '>');
  foreach (entity_load('ideal_acquirer') as $acquirer) {
    $options[$acquirer->iaid] = $acquirer->title;
  }
  $form['iaid'] = array(
    '#type' => 'select',
    '#title' => t('Acquirer'),
    '#default_value' => $merchant ? $merchant->acquirer->iaid : 0,
    '#options' => $options,
    '#required' => TRUE,
  );
  $form['merchant_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant ID'),
    '#default_value' => $merchant ? $merchant->merchant_id : NULL,
    '#maxlength' => 9,
    '#size' => 9,
    '#required' => TRUE,
  );
  $form['sub_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Sub ID'),
    '#default_value' => $merchant ? $merchant->sub_id : NULL,
    '#maxlength' => 6,
    '#size' => 6,
  );
  $form['private_key_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to private key file'),
    '#default_value' => $merchant ? $merchant->private_key_path : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['private_key_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Private key password'),
    '#default_value' => $merchant ? $merchant->private_key_password : NULL,
    '#description' => t('This password will be stored as plain text.'),
    '#maxlength' => 24,
  );
  $form['public_certificate_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to public certificate file'),
    '#default_value' => $merchant ? $merchant->public_certificate_path : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if ($merchant && idealui_access_merchant($merchant, 'delete')) {
    $form['actions']['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => 'admin/config/services/ideal/merchant/delete/' . $merchant->imid,
    );
  }

  return $form;
}

/**
 * Implements form validate callback for idealui_form_merchant().
 */
function idealui_form_merchant_validate(array $form, array &$form_state) {
  $values = $form_state['values'];
  if (preg_match('#[^\da-z]#i', $values['merchant_id'])) {
    form_set_error('merchant_id', t('The merchant ID must be alphanumeric.'));
  }
  if (preg_match('#[\D]#i', $values['sub_id'])) {
    form_set_error('sub_id', t('The sub ID must be numeric.'));
  }
  if ($values['iaid'] == 0) {
    form_set_error('acquirer', t('Acquirer field is required.'));
  }
  if (!is_readable($values['private_key_path'])) {
    form_set_error('private_key_path', t('The private key file is not readable.'));
  }
  elseif (!@openssl_pkey_get_private('file://' . $values['private_key_path'], $values['private_key_password'])) {
    form_set_error('private_key_path', t('The private key could not be extracted from the private key file.'));
  }
  if (!is_readable($values['public_certificate_path'])) {
    form_set_error('public_certificate_path', t('The public certificate file is not readable.'));
  }
  else {
    $certificate = NULL;
    if (!($x509 = @openssl_x509_read(file_get_contents($values['public_certificate_path']))) || !@openssl_x509_export($x509, $certificate)) {
      form_set_error('public_certificate_path', t('The public certificate could not be extracted from the public certificate file.'));
    }
  }
  // Validate the key and certificate against eachother.
  $errors = form_get_errors();
  if (!isset($errors['private_key_path']) && !isset($errors['public_certificate_path'])) {
    $acquirer = new iDEALAcquirer(array());
    $token_values = array('foo', 'bar');
    $token_code = $acquirer->tokenCode($token_values, $values['private_key_path'], $values['private_key_password']);
    if (!$acquirer->verifyTokenCode($token_values, $token_code, $values['public_certificate_path'])) {
      form_set_error('private_key_path', t('The private key and public certificate do not match.'));
    }
  }
}

/**
 * Implements form submit callback for idealui_form_merchant().
 */
function idealui_form_merchant_submit(array $form, array &$form_state) {
  $values = $form_state['values'];
  $merchant = new iDEALMerchant(array(
    'imid' => $values['imid'],
    'title' => $values['title'],
    'merchant_id' => $values['merchant_id'],
    'sub_id' => $values['sub_id'],
    'acquirer' => entity_load_single('ideal_acquirer', $values['iaid']),
    'private_key_path' => $values['private_key_path'],
    'private_key_password' => $values['private_key_password'],
    'public_certificate_path' => $values['public_certificate_path'],
  ));
  entity_save('ideal_merchant', $merchant);
  $form_state['redirect'] = 'admin/config/services/ideal/merchant';
  drupal_set_message(t('The merchant %title has been saved.', array(
    '%title' => $merchant->title,
  )));
}

/**
 * Implements form build callback: merchant deletion form.
 */
function idealui_form_merchant_delete(array $form, array &$form_state, iDEALMerchant $merchant) {
  $form['imid'] = array(
    '#type' => 'value',
    '#value' => $merchant->imid,
  );
  return confirm_form($form, t('Do you really want to delete merchant %title?', array(
    '%title' => $merchant->title,
  )), 'admin/config/services/ideal/merchant');
}

/**
 * Implements form submit callback for idealui_form_merchant_delete().
 */
function idealui_form_merchant_delete_submit(array $form, array &$form_state) {
  $imid = $form_state['values']['imid'];
  $merchant = entity_load_single('ideal_merchant', $imid);
  entity_delete('ideal_merchant', $imid);
  $form_state['redirect'] = 'admin/config/services/ideal/merchant';
  drupal_set_message(t('The merchant %title has been deleted.', array(
    '%title' => $merchant->title,
  )));
}

/**
 * Implements form build callback: make a merchant the default one.
 */
function idealui_form_merchant_default(array $form, array &$form_state, iDEALMerchant $merchant) {
  $form['imid'] = array(
    '#type' => 'value',
    '#value' => $merchant->imid,
  );

  return confirm_form($form, t('Do you really want to make %merchant_title the default merchant?', array(
    '%merchant_title' => $merchant->title,
  )), 'admin/config/services/ideal/merchant', t("The default merchant will be used by modules that aren't configured to use a specific merchant."), t('Make default'));
}

/**
 * Implements form submit callback for idealui_form_merchant_default().
 */
function idealui_form_merchant_default_submit(array $form, array &$form_state) {
  $imid = $form_state['values']['imid'];
  variable_set('ideal_default_imid', $imid);
  $merchant = entity_load_single('ideal_merchant', $imid);
  drupal_set_message(t('%merchant_title has been set as the default merchant.', array(
    '%merchant' => $merchant->title,
  )));
  $form_state['redirect'] = 'admin/config/services/ideal/merchant';
}

/**
 * The acquirer overview.
 */
function idealui_acquirer_list() {
  $rows = array();
  foreach (entity_load('ideal_acquirer') as $acquirer) {
    $rows[] = array(
      $acquirer->title,
      idealui_access_acquirer($acquirer, 'edit') ? l(t('edit'), 'admin/config/services/ideal/acquirer/edit/' . $acquirer->iaid) : NULL,
      idealui_access_acquirer($acquirer, 'delete') ? l(t('delete'), 'admin/config/services/ideal/acquirer/delete/' . $acquirer->iaid) : NULL,
    );
  }
  $header = array(
    t('Title'),
    array(
      'data' => t('Operations'),
      'colspan' => 2,
    ),
  );

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
  ));
}

/**
 * Implements form build callback: the acquirer add/edit form.
 *
 * @param $acquirer mixed
 *   Either an iDEALAcquirer object to edit it, or NULL to display an empty
 *   form to add a new acquirer.
 */
function idealui_form_acquirer(array $form, array &$form_state, iDEALAcquirer $acquirer = NULL) {
  $form['iaid'] = array(
    '#type' => 'value',
    '#value' => $acquirer ? $acquirer->iaid : 0,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $acquirer ? $acquirer->title : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['url_live'] = array(
    '#type' => 'textfield',
    '#title' => t('Production server URL'),
    '#default_value' => $acquirer ? $acquirer->url_live : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['url_test'] = array(
    '#type' => 'textfield',
    '#title' => t('Testing server URL'),
    '#default_value' => $acquirer ? $acquirer->url_test : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['public_certificate_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to public certificate file'),
    '#default_value' => $acquirer ? $acquirer->public_certificate_path : NULL,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['aactions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if ($acquirer && idealui_access_acquirer($acquirer, 'delete')) {
    $form['actions']['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => 'admin/config/services/ideal/acquirer/delete/' . $acquirer->iaid,
    );
  }

  return $form;
}

/**
 * Implements form validate callback for idealui_form_acquirer().
 */
function idealui_form_acquirer_validate(array $form, array &$form_state) {
  $values = $form_state['values'];
  if (!valid_url($values['url_live'], TRUE)) {
    form_set_error('url_live', t('The production server URL is invalid.'));
  }
  if (!valid_url($values['url_test'], TRUE)) {
    form_set_error('url_live', t('The testing server URL is invalid.'));
  }
  if (!is_readable($values['public_certificate_path'])) {
    form_set_error('public_certificate_path', t('The public certificate file is not readable.'));
  }
  elseif (!@openssl_pkey_get_public('file://' . $values['public_certificate_path'])) {
    form_set_error('public_certificate_path', t('The public certificate could not be extracted from the public certificate file.'));
  }
}

/**
 * Implements form submit callback for idealui_form_acquirer().
 */
function idealui_form_acquirer_submit(array $form, array &$form_state) {
  module_load_include('inc', 'ideal', '/includes/ideal.transaction');
  $values = $form_state['values'];
  $acquirer = new iDEALAcquirer(array(
    'iaid' => $values['iaid'],
    'title' => $values['title'],
    'url_live' => $values['url_live'],
    'url_test' => $values['url_test'],
    'public_certificate_path' => $values['public_certificate_path'],
  ));
  entity_save('ideal_acquirer', $acquirer);
  $form_state['redirect'] = 'admin/config/services/ideal/acquirer';
  drupal_set_message(t('The acquirer %title has been saved.', array(
    '%title' => $acquirer->title,
  )));
}

/**
 * Implements form build callback: acquirer deletion form.
 *
 * @param $acquirer iDEALAcquirer
 *   The acquirer to delete.
 */
function idealui_form_acquirer_delete(array $form, array &$form_state, iDEALAcquirer $acquirer) {
  $form['iaid'] = array(
    '#type' => 'value',
    '#value' => $acquirer->iaid,
  );
  return confirm_form($form, t('Do you really want to delete acquirer %title?', array(
    '%title' => $acquirer->title,
  )), 'admin/config/services/ideal/acquirer');
}

/**
 * Implements form submit callback for idealui_form_acquirer_delete().
 */
function idealui_form_acquirer_delete_submit(array $form, array &$form_state) {
  $iaid = $form_state['values']['iaid'];
  $acquirer = entity_load_single('ideal_acquirer', $iaid);
  entity_delete('ideal_acquirer', $iaid);
  $form_state['redirect'] = 'admin/config/services/ideal/acquirer';
  drupal_set_message(t('The acquirer %title has been deleted.', array(
    '%title' => $acquirer->title,
  )));
}

/**
 * Implements form build callback: global configuration form.
 */
function idealui_form_global_configuration(array $form, array &$form_state) {
  $form['ideal_test'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use testing servers only'),
    '#default_value' => variable_get('ideal_test', TRUE),
  );
  $form['ideal_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display log errors on screen'),
    '#default_value' => variable_get('ideal_debug', TRUE),
  );
  $form['ideal_default_expiration_period'] = array(
    '#type' => 'select',
    '#title' => t('Default expiration period'),
    '#options' => array(
      180 => t('3 minutes'),
      240 => t('4 minutes'),
      300 => t('5 minutes'),
      600 => t('10 minutes'),
      900 => t('15 minutes'),
      1200 => t('20 minutes'),
      1800 => t('30 minutes'),
      2400 => t('40 minutes'),
      3000 => t('50 minutes'),
      3600 => t('1 hour'),
    ),
    '#default_value' => variable_get('ideal_default_expiration_period', 900),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}