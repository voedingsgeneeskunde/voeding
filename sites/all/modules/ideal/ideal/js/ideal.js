(function($) {
  /**
   * Initialize the pay button's behavior.
   */
  Drupal.behaviors.iDEALPayButton = {
    attach: function(context) {
      Drupal.settings.iDEALPayButton = $('#edit-pay').val();
      Drupal.iDEALPayButton();
      $('#edit-issuer').bind('change', function(event) {
        Drupal.iDEALPayButton();
      })
    }
  }

  /**
   * Update the pay button's text based on the selected bank.
   */
  Drupal.iDEALPayButton = function() {
    if (!$.inArray($('#edit-issuer').val(), ['ideal_choose', 'ideal_choose_other'])) {
      $('#edit-pay').val(Drupal.settings.iDEALPayButton).attr('disabled', 'disabled');
    }
    else {
      arguments['@bank'] = $('#edit-issuer option:selected').text();
      $('#edit-pay').val(Drupal.t('Pay via @bank', arguments)).removeAttr('disabled');
    }
  }
})(jQuery);