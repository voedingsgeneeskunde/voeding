<?php

/**
 * @file
 * The API and related functions for executing and managing transactions.
 */

/**
 * Base class with common functionality.
 */
class iDEALBase {
  function __construct(array $properties) {
    foreach ($properties as $property => $value) {
      $this->$property = $value;
    }
  }
}

/**
 * An iDEAL transaction. Contains all transaction-specific data.
 */
class iDEALTransaction extends iDEALBase {

  /**
   * The payer's bank account number.
   *
   * @var integer
   */
  public $account_number = NULL;

  /**
   * The transaction amount.
   *
   * @var integer
   */
  public $amount = 0;

  /**
   * The city the payer lives in.
   *
   * @var string
   */
  public $city = NULL;

  /**
   * The Unix datetime the transaction request was created.
   *
   * @var integer
   */
  public $created = 0;

  /**
   * The transaction description.
   *
   * @var string
   */
  public $description = NULL;

  /**
   * The entrance code, which should be NULL if $status != IDEAL_STATUS_OPEN.
   *
   * @var string
   */
  public $entrance_code = NULL;

  /**
   * The Unix datetime the transaction expires.
   *
   * @var integer
   */
  public $expired = NULL;

  /**
   * The URL as which the user can approve the transaction.
   *
   * @var string
   */
  public $issuer_authentication_url = NULL;

  /**
   * The internal ID of this transaction.
   *
   * @var integer
   */
  public $itid = 0;

  /**
   * The merchant that is used to make this transaction.
   *
   * @var iDEALMerchant
   */
  public $merchant = NULL;

  /**
   * The payer's real name.
   *
   * @var string
   */
  public $name = NULL;

  /**
   * The purchase ID.
   *
   * @var string
   */
  public $purchase_id = NULL;

  /**
   * A callback function to be executed when the payer returns from his bank's
   * iDEAL environment. The iDEALTransaction object is passed on as the first
   * parameter. The callback should either return a renderable array or a
   * string as page output, or redirect the user.
   *
   * @var string
   */
  public $return_callback = NULL;

  /**
   * The current status.
   *
   * @see iDEALTransaction::statusUpdate()
   *
   * @var string
   */
  public $status = IDEAL_STATUS_OPEN;

  /**
   * The transaction ID.
   *
   * @var integer
   */
  public $transaction_id = NULL;

  /**
   * The UID of the user that initiated this transaction.
   *
   * @var integer
   */
  public $uid = 0;

  /**
   * The Unix datetime the transaction was last updated.
   *
   * @var integer
   */
  public $updated = 0;

  /**
   * Constructor.
   *
   * @param $properties array
   *   An associative array. Keys are property names and values are property
   *   values.
   *
   * @return NULL
   */
  function __construct(array $properties = array()) {
    global $user;

    $this->uid = $user->uid;
    $this->created = time();
    parent::__construct($properties);
    if (!$this->merchant) {
      if ($this->imid) {
        $this->merchant = entity_load_single('ideal_merchant', $this->imid);
      }
      else {
        $this->merchant = entity_load_single('ideal_merchant', variable_get('ideal_default_imid', 1));
      }
    }
    if (!$this->expired) {
      $this->expired = $this->created + variable_get('ideal_default_expiration_period', 900);
    }
  }

  /**
   * Return the entrance code for a transaction request.
   *
   * @return string
   */
  function entranceCode() {
    if (!$this->entrance_code) {
      $characters = 'abcdefghijklmnoprstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
      for ($i = 0; $i < 40; $i++) {
        $this->entrance_code .= $characters[mt_rand(0, strlen($characters) - 1)];
      }
    }

    return $this->entrance_code;
  }

  /**
   * Execute the actual transaction.
   *
   * @see iDEALAcquirer::_transactionRequest()
   *
   * @param $issuer_id string
   *   The issuing bank's ID, with leading zeroes until the string is 4
   *   characters long.
   *
   * @return mixed
   *   In case of success, the issuer authentication URL.
   */
  function execute($issuer_id) {
    if ($info = $this->merchant->acquirer->_transactionRequest($this, $issuer_id)) {
      $this->transaction_id = $info['transaction_id'];
      $this->issuer_authentication_url = $info['issuer_authentication_url'];
      entity_save('ideal_transaction', $this);
      return $this->issuer_authentication_url;
    }
    $this->status = IDEAL_STATUS_FAILURE;
    call_user_func($this->return_callback, $this);
  }

  /**
   * Check the transaction's current status with the bank.
   *
   * @see iDEALAcquirer::_statusRequest()
   *
   * @return boolean
   *   Whether the status check was successful.
   */
  function statusUpdate() {
    if ($this->status == IDEAL_STATUS_OPEN && $info = $this->merchant->acquirer->_statusRequest($this)) {
      $this->status = $info['status'];
      if ($this->status == IDEAL_STATUS_SUCCESS) {
        $this->name = $info['name'];
        $this->city = $info['city'];
        $this->account_number = $info['account_number'];
      }
      module_invoke_all('ideal_transaction_close', $this);
      return TRUE;
    }
    return FALSE;
  }
}

/**
 * Entity API controller for ideal_transaction entities.
 */
class iDEALTransactionEntityController extends EntityAPIController {

  /**
   * Implement EntityAPIController#save().
   */
  function save($entity, DatabaseTransaction $transaction = NULL) {
    $entity->updated = time();
    if ($entity->status != IDEAL_STATUS_OPEN) {
      $entity->entrance_code = '';
    }

    // Save the merchant as its IMID.
    $entity->imid = $entity->merchant->imid;

    return parent::save($entity, $transaction);
  }
}

/**
 * Merchant configuration.
 */
class iDEALMerchant extends iDEALBase {

  /**
   * The acquirer this merchant uses.
   *
   * @var iDEALAcquirer
   */
  public $acquirer = NULL;

  /**
   * The IAID of $this->acquirer. Only used for storage purposes.
   *
   * @var integer
   */
  public $iaid = 0;

  /**
   * The internal ID of this merchant.
   *
   * @var integer
   */
  public $imid = 0;

  /**
   * Whether this merchant is locked and cannot be edited.
   *
   * @var boolean
   */
  public $locked = FALSE;

  /**
   * The merchant ID as provided by the bank.
   *
   * @var string
   */
  public $merchant_id = NULL;

  /**
   * The path to the public certificate file.
   *
   * @var integer
   */
  public $public_certificate_path = NULL;

  /**
   * The private key password.
   *
   * @var string
   */
  public $private_key_password = NULL;

  /**
   * The path to the private key file.
   *
   * @var string
   */
  public $private_key_path = NULL;

  /**
   * The sub ID.
   *
   * @var integer
   */
  public $sub_id = 0;

  /**
   * The human-readable title.
   *
   * @var string
   */
  public $title = NULL;

  /**
   * Constructor.
   *
   * @param $properties array
   *   An associative array. Keys are property names and values are property
   *   values.
   *
   * @return NULL
   */
  function __construct(array $properties = array()) {
    parent::__construct($properties);
    if (!$this->acquirer && $this->iaid) {
      $this->acquirer = entity_load_single('ideal_acquirer', $this->iaid);
    }
  }
}

/**
 * Entity API controller for ideal_merchant entities.
 */
class iDEALMerchantEntityController extends EntityAPIController {

  /**
   * Implement EntityAPIController#save().
   */
  function save($entity, DatabaseTransaction $transaction = NULL) {
    // Save the acquirer as its IAID.
    $entity->iaid = $entity->acquirer->iaid;

    return parent::save($entity, $transaction);
  }
}

/**
 * Acquirer configuration.
 */
class iDEALAcquirer extends iDEALBase {

  /**
   * The acquirer's internal ID.
   *
   * @var integer
   */
  public $iaid = 0;

  /**
   * Whether this acquirer is locked and cannot be edited.
   *
   * @var boolean
   */
  public $locked = FALSE;

  /**
   * The path to the acquirer's public certificate file.
   *
   * @var integer
   */
  public $public_certificate_path = NULL;

  /**
   * The acquirer's human-readable title.
   *
   * @var string
   */
  public $title = NULL;

  /**
   * The URL of the live/production server.
   *
   * @var string
   */
  public $url_live = NULL;

  /**
   * The URL of the testing/staging server.
   *
   * @var string
   */
  public $url_test = NULL;

  /**
   * Prepare a description for a request.
   *
   * @param $description string
   *
   * @return string
   */
  function description($description) {
    // Convert diacritics to HTML entities and replace illegal characters with
    // underscores.
    return substr(trim(preg_replace('#[^a-z0-9=[:space:]%*+,-./&@"\':;?()$]#i', '_', htmlentities($description, ENT_NOQUOTES, 'UTF-8'))), 0, 32);
  }

  /**
   * Construct the XML for a request.
   *
   * @param $fields array
   *   A nested array representing the XML tree. Keys are tags, values are
   *   tags' contents.
   *
   * @return string
   *   XML code.
   */
  private function xml(array $fields) {
    $xml = '';
    foreach ($fields as $name => $value) {
      $xml .= "<$name>";
      if (is_array($value)) {
        $value = $this->xml($value);
      }
      $xml .= "$value</$name>";
    }

    return $xml;
  }

  /**
   * Create a tokenCode (signature) for a request.
   *
   * @param $values array
   *   The values to create a tokenCode from.
   * @param $private_key_path string
   *   The path to the file that contains the merchant's private key.
   * @param $private_key_password string
   *   The password with which to decrypt the merchant's private key.
   *
   * @return mixed
   *   The tokenCode as a string or FALSE in case of failure.
   */
  function tokenCode(array $values, $private_key_path, $private_key_password = NULL) {
    $private_key = openssl_pkey_get_private('file://' . $private_key_path, $private_key_password);
    if ($private_key) {
      $data = preg_replace('#\s#', '', implode($values));
      $token_code = NULL;
      openssl_sign($data, $token_code, $private_key);
      openssl_free_key($private_key);
      return base64_encode($token_code);
    }
    watchdog('iDEAL', 'The private key could not be extracted from private key file %file.', array(
      '%file' => $private_key_path,
    ), WATCHDOG_ERROR);
    return FALSE;
  }

  /**
   * Verify a received tokenCode (signature) for a response.
   *
   * @param $values array
   *   The values the tokenCode was created from.
   * @param $token_code string
   *   The tokenCode to check.
   * @param $public_certificate_path string
   *   The path to the file that contains the acquirer's public certificate.
   *
   * @return boolean
   *   TRUE if the tokenCode is valid. FALSE if the tokenCode is invalid or
   *   verification could not be done.
   */
  function verifyTokenCode($values, $token_code, $public_certificate_path) {
    $public_key = openssl_pkey_get_public('file://' . $public_certificate_path);
    if ($public_key) {
      $data = preg_replace('#\s#', '', implode($values));
      $return = (bool) openssl_verify($data, base64_decode($token_code), $public_key);
      openssl_free_key($public_key);
      return $return;
    }
    watchdog('iDEAL', 'The public certificate could not be extracted from public certificate file %file.', array(
      '%file' => $public_certificate_path,
    ), WATCHDOG_ERROR);
    return FALSE;
  }

  /**
   * Create the certificate's fingerprint for a request's token field.
   *
   * @param $public_certificate_path string
   *   The path to the file that contains the merchant's public certificate.
   *
   * @return string
   */
  function token($public_certificate_path) {
    $certificate = NULL;
    if (openssl_x509_export(openssl_x509_read(file_get_contents($public_certificate_path)), $certificate)) {
      $certificate = str_replace(array('-----BEGIN CERTIFICATE-----', '-----END CERTIFICATE-----'), '', $certificate);
      $certificate = base64_decode($certificate);
      $fingerprint = sha1($certificate);
      return strtoupper($fingerprint);
    }
    watchdog('iDEAL', 'The public certificate could not be extracted from public certificate file file %file.', array(
      '%file' => $public_certificate_path,
    ), WATCHDOG_ERROR);
    return FALSE;
  }

  /**
   * Construct the XML for a request.
   *
   * @param $merchant iDEALMerchant
   *   The merchant to use for this request.
   * @param $request_type string
   *   Either 'Directory', 'AcquirerTrx' (transaction), or 'AcquirerStatus'.
   * @param $token_code_values array
   *   An array of field values required to generate the token code.
   * @param $fields array
   *   A nested array representing the XML tree. Keys are tags, values are
   *   tags' contents.
   *
   * @return mixed
   *   The response data or FALSE if the request failed.
   */
  function request(iDEALMerchant $merchant, $request_type, array $token_code_values, array $fields = array()) {
    // Prefix the merchant ID with zeroes until it is 9 characters long.
    $merchant_id = str_repeat('0', 9 - strlen($merchant->merchant_id)) . $merchant->merchant_id;
    $datetimestamp = gmdate('o-m-d\TH:i:s.000\Z', time());
    array_unshift($token_code_values, $datetimestamp);
    $defaults = array(
      'createDateTimeStamp' => $datetimestamp,
      'Merchant' => array(
        'merchantID' => $merchant_id,
        'subID' => $merchant->sub_id,
        'authentication' => 'SHA1_RSA',
        'token' => $this->token($merchant->public_certificate_path),
        'tokenCode' => $this->tokenCode($token_code_values, $merchant->private_key_path, $merchant->private_key_password),
      ),
    );
    $fields = array_merge_recursive($fields, $defaults);
    $xml = '<?xml version="1.0" encoding="UTF-8"?>';
    $xml .= '<' . $request_type . 'Req xmlns="http://www.idealdesk.com/Message" version="1.1.0">';
    $xml .= $this->xml($fields);
    $xml .= '</' . $request_type . 'Req>';

    $url = variable_get('ideal_test', TRUE) ? $this->url_test : $this->url_live;
    $response = drupal_http_request($url, array(
      'headers' => array(
        'content-type' => 'text/xml; charset="UTF-8"',
      ),
      'method' => 'POST',
      'data' => $xml,
    ));

    // The HTTP request went wrong.
    if (isset($response->error)) {
      watchdog('iDEAL', '@error_message (@error_code) (!request_type)', array(
        '@error_message' => $response->error,
        '@error_code' => $response->code,
        '!request_type' => $request_type,
      ), WATCHDOG_ERROR);
      return FALSE;
    }
    $dom = @simplexml_load_string($response->data);
    // The request did not return XML.
    if (!$dom) {
      watchdog('iDEAL', 'The iDEAL server at %url did not return valid XML.', array(
        '%url' => $url,
      ), WATCHDOG_ERROR);
      return FALSE;
    }
    // The iDEAL server encountered an error.
    if (isset($dom->Error)) {
      watchdog('iDEAL', '@error_message (@error_code) (!request_type)', array(
        '@error_message' => (string) $dom->Error->errorMessage . '; ' . (string) $dom->Error->errorDetail,
        '@error_code' => (string) $dom->Error->errorCode,
        '!request_type' => $request_type,
      ), WATCHDOG_ERROR);
      return FALSE;
    }
    // Everything went allright.
    return $response->data;
  }

  /**
   * Execute a directory request.
   *
   * @see iDEALAcquirer::issuers()
   *
   * @param $merchant iDEALMerchant
   *   The merchant to use for this request.
   *
   * @return array
   *  Should be of the form array(
   *    'short' => array(
   *      'issuerID' => 'issuerName',
   *    ),
   *    'long' => array(
   *      'issuerID' => 'issuerName',
   *    ),
   *  );
   */
  function _directoryRequest(iDEALMerchant $merchant) {
    $token_code_values = array(
      $merchant->merchant_id,
      $merchant->sub_id,
    );
    if ($xml = $this->request($merchant, 'Directory', $token_code_values, array())) {
      $dom = simplexml_load_string($xml);
      $issuers = array(
        'short' => array(),
        'long' => array(),
      );
      foreach ($dom->Directory->Issuer as $issuer) {
        $issuers[strtolower((string) $issuer->issuerList)][(string) $issuer->issuerID] = (string) $issuer->issuerName;
      }
      return $issuers;
    }
    return FALSE;
  }

  /**
   * Return a list of issuing banks.
   *
   * @see iDEALAcquirer::_directoryRequest()
   *
   * @param $merchant iDEALMerchant
   *   The merchant to use for this request.
   *
   * @return array
   *   A list of issuers, identical to the output of
   *   iDEALAcquirer::_directoryRequest().
   */
  function issuers(iDEALMerchant $merchant) {
    $cid = 'ideal_issuers_' . $merchant->imid . '_' . (variable_get('ideal_test', TRUE) ? 'test' : 'live');
    $cache = cache_get($cid);
    if ($cache) {
      return $cache->data;
    }
    else {
      $issuers = $this->_directoryRequest($merchant);
      // Cache issuers for 24h in case of success. If the request fails, cache
      // the result for ten minutes to prevent errors from repeatedly popping
      // up.
      $lifetime = 86400;
      if (!$issuers || empty($issuers['short'])) {
        $lifetime = 600;
        watchdog('iDEAL', 'iDEAL could not retrieve the list of issuing banks and will try again in ten minutes. Clear the cache to force a retry immediately.', NULL, WATCHDOG_ERROR);
      }
      cache_set($cid, $issuers, 'cache', time() + $lifetime);
      return $issuers;
    }
  }

  /**
   * Execute a transaction request.
   *
   * @see iDEALTransaction::execute()
   *
   * @param $transaction iDEALTransaction
   *   The transaction to execute a request for.
   * @param $issuer_id string
   *   The issuing bank's ID, with leading zeroes until the string is 4
   *   characters long.
   *
   * @return mixed
   *   A keyed array with transaction data provided by the bank:
   *   - transaction_id: The transaction ID.
   *   - issuer_authentication_url: The issuer authentication URL.
   *   In case of errors, FALSE is returned.
   */
  function _transactionRequest(iDEALTransaction $transaction, $issuer_id) {
    global $language;

    // Check for required data. Assume merchant configuration is in order.
    foreach (array('amount', 'description', 'purchase_id') as $property) {
      if (!$transaction->$property) {
        watchdog('iDEAL', 'Missing required transaction property %property.', array(
          '%property' => '$transaction->' . $property,
        ), WATCHDOG_ERROR);
        return FALSE;
      }
    }

    $issuer_id = str_repeat('0', 4 - strlen($issuer_id)) . $issuer_id;
    $merchant_id = str_repeat('0', 9 - strlen($transaction->merchant->merchant_id)) . $transaction->merchant->merchant_id;
    $description = $this->description($transaction->description);
    $purchase_id = drupal_substr($transaction->purchase_id, 0, 16);
    $token_code_fields = array(
      $issuer_id,
      $merchant_id,
      $transaction->merchant->sub_id,
      ideal_merchant_return_url(),
      $purchase_id,
      $transaction->amount,
      'EUR',
      $language->language,
      $description,
      $transaction->entranceCode(),
    );
    $fields = array(
      'Issuer' => array(
        'issuerID' => $issuer_id,
      ),
      'Merchant' => array(
        'merchantReturnURL' => ideal_merchant_return_url(),
      ),
      'Transaction' => array(
        'purchaseID' => $purchase_id,
        'amount' => $transaction->amount,
        'currency' => 'EUR',
        'expirationPeriod' => ideal_secondstoiso8601($transaction->expired - $transaction->created),
        'language' => $language->language,
        'description' => $description,
        'entranceCode' => $transaction->entranceCode(),
      ),
    );

    if ($xml = $this->request($transaction->merchant, 'AcquirerTrx', $token_code_fields, $fields)) {
      $dom = simplexml_load_string($xml);
      return array(
        'transaction_id' => (string) $dom->Transaction->transactionID,
        'issuer_authentication_url' => (string) $dom->Issuer->issuerAuthenticationURL,
      );
    }
    return FALSE;
  }

  /**
   * Execute a status request.
   *
   * @see iDEALTransaction::statusUpdate()
   *
   * @param $transaction iDEALTransaction
   *   The transaction to check the status for.
   *
   * @return mixed
   *   A keyed array with transaction data provided by the bank:
   *   - status: The transaction status.
   *   - name: The payer's name.
   *   - city: The payer's registered place of residence.
   *   - account_number: the payer's bank account number.
   *   In case of errors, FALSE is returned.
   */
  function _statusRequest(iDEALTransaction $transaction) {
    $merchant_id = str_repeat('0', 9 - strlen($transaction->merchant->merchant_id)) . $transaction->merchant->merchant_id;
    $token_code_fields = array(
      $merchant_id,
      $transaction->merchant->sub_id,
      $transaction->transaction_id,
    );
    $fields = array(
      'Transaction' => array(
        'transactionID' => $transaction->transaction_id,
      ),
    );

    if ($xml = $this->request($transaction->merchant, 'AcquirerStatus', $token_code_fields, $fields)) {
      $dom = simplexml_load_string($xml);

      // Verify the response's authenticity.
      $token_code_values = array(
        (string) $dom->createDateTimeStamp,
        (string) $dom->Transaction->transactionID,
        (string) $dom->Transaction->status,
      );
      $status = (string) $dom->Transaction->status;
      // The account number is only added in case of success.
      if ($status == IDEAL_STATUS_SUCCESS) {
        $token_code_values[] = (string) $dom->Transaction->consumerAccountNumber;
      }
      if ($this->verifyTokenCode($token_code_values, (string) $dom->Signature->signatureValue, $transaction->merchant->acquirer->public_certificate_path)) {
        // The response is authentic. Return status information.
        return array(
          'status' => $status,
          'name' => (string) $dom->Transaction->consumerName,
          'city' => (string) $dom->Transaction->consumerCity,
          'account_number' => (string) $dom->Transaction->consumerAccountNumber,
        );
      }
    }
    return FALSE;
  }
}

/**
 * Implements form build callback: display a transaction form.
 *
 * @param $transaction iDEALTransaction
 * @param $description string
 *   A description to show to the user about the transaction that he is about
 *   to make.
 * @param $minimum_amount mixed
 *   An integer that represents the minimum amount of cents a user needs to
 *   enter or FALSE to use a fixed transaction amount.
 *
 * @return array
 *   A Drupal form.
 */
function ideal_form_transaction(array $form, array $form_state, iDEALTransaction $transaction, $description, $minimum_amount = FALSE) {
  $form = array();
  if (!($issuers = $transaction->merchant->acquirer->issuers($transaction->merchant))) {
    drupal_set_message(t('The list of available banks could not be retrieved.'), 'error');
    $form['#access'] = FALSE;
  }

  // Explicitely set form handlers to make sure they will be called, even if
  // the form is embedded in another form.
  $form['#submit'] = array('ideal_form_transaction_submit');
  $form['#theme'] = 'ideal_form_transaction';

  $form['transaction'] = array(
    '#type' => 'value',
    '#value' => $transaction,
  );
  $form['description'] = array(
    '#markup' => $description,
  );
  if (is_int($minimum_amount)) {
    $form['amount'] = array(
      '#type' => 'ideal_transaction_amount',
      '#title' => t('Amount'),
      '#amount' => $transaction->amount,
      '#minimum_amount' => $minimum_amount,
      '#access' => $minimum_amount !== FALSE,
    );
  }
  else {
    $form['amount'] = array(
      '#type' => 'value',
      '#value' => $transaction->amount,
    );
    $form['amount_human_readable'] = array(
      '#type' => 'item',
      '#title' => t('Amount'),
      '#markup' => '€' . ideal_cents_to_human_readable($transaction->amount),
    );
  }
  $options = array(
    'ideal_choose' => t('- Select -'),
  );
  if (isset($issuers['short'])) {
    foreach ($issuers['short'] as $issuer_id => $issuer_name) {
      $options[$issuer_id] = $issuer_name;
    }
  }
  if (!empty($issuers['long'])) {
    $options['ideal_choose_other'] = '---' . t('Other banks') . '---';
    foreach ($issuers['long'] as $issuer_id => $issuer_name) {
      $options[$issuer_id] = $issuer_name;
    }
  }
  $form['issuer'] = array(
    '#type' => 'select',
    '#title' => t('Choose your bank'),
    '#options' => $options,
    '#required' => TRUE,
    '#default_value' => 'ideal_choose',
    '#element_validate' => array('ideal_form_validate_issuer'),
  );
  $form['return'] = array(
    '#value' =>  t('You will automatically return to @site_name after completing the transaction.', array(
      '@site_name' => variable_get('site_name', 'Drupal'),
    )),
  );
  if (variable_get('ideal_test', TRUE)) {
    $form['test'] = array(
      '#value' =>  t('This is a test transaction. No actual money will be transferred.'),
    );
  }
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['pay'] = array(
    '#type' => 'submit',
    '#value' => t('Pay via my bank'),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'ideal') . '/js/ideal.js'),
    ),
  );

  return $form;
}

/**
 * Implements form submit callback for ideal_form_transaction().
 */
function ideal_form_transaction_submit(array $form, array &$form_state) {
  $values = $form_state['values'];
  $transaction = $values['transaction'];
  $transaction->amount = $values['amount'];
  if ($url = $transaction->execute($values['issuer'])) {
    drupal_goto($url);
  }
}

/**
 * Implements theme function for ideal_form_transaction().
 */
function theme_ideal_form_transaction(array $variables) {
  $form = $variables['form'];

  $test = isset($form['test']) ? '<div class="messages warning">' . $form['test']['#value'] . '</div>' : NULL;
  $form['test']['#printed'] = TRUE;
  $actions = drupal_render($form['actions']);
  $return = '<p class="description">' . $form['return']['#value'] . '</p>';
  $form['return']['#printed'] = TRUE;

  return drupal_render_children($form) . $test . $actions . $return;
}

/**
 * Process a user who just returned from his issuer's website after a transaction.
 *
 * @return mixed
 *   This function either terminates the request by displaying a "page not
 *   found" page or by sending a location header, or it returns a renderable
 *   array or a string as page output.
 */
function ideal_transaction_return() {
  $query = new EntityFieldQuery();
  $entities = $query->entityCondition('entity_type', 'ideal_transaction')
    ->propertyCondition('transaction_id', $_GET['trxid'])
    ->propertyCondition('entrance_code', $_GET['ec'])
    ->propertyCondition('status', IDEAL_STATUS_OPEN)
    ->execute();
  $ids = array_keys($entities['ideal_transaction']);
  $transaction = entity_load_single('ideal_transaction', reset($ids));

  if (!$transaction) {
    drupal_access_denied();
    exit;
  }

  $transaction->statusUpdate();
  entity_save('ideal_transaction', $transaction);

  return call_user_func($transaction->return_callback, $transaction);
}