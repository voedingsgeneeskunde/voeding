<?php

/**
 * @file
 * Hook documentation.
 */

/**
 * Act when a transaction has just been closed e.g. its status changed from
 * IDEAL_STATUS_OPEN to another one.
 *
 * @param $transaction iDEALTransaction
 *   The transaction that has been closed. The updated status has already been
 *   set.
 *
 * @return NULL
 */
function hook_ideal_transaction_close(iDEALTransaction $transaction) {
  if ($transaction->status == IDEAL_STATUS_SUCCESS) {
    // Send an email to the site administrator to tell him the transaction was
    // successful.
  }
}