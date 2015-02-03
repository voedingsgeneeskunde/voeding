ABOUT
-----

Allows users to make transactions using the iDEAL bank transfer system for Dutch
banks.

- iDEAL API is an API to connect Drupal to the iDEAL servers of various
  banks. This means that it doesn't do anything by itself, apart from providing
  the ability to make transactions to other modules.
- iDEAL Donation Form contains a simple payment form.
- iDEAL Administration Interface contains the configuration interface and
  transaction overview.


GENERAL CONFIGURATION
---------------------

iDEAL Administration Interface offers configuration pages at Administer >
Configuration > Web services > iDEAL API where you can enter your merchant
information and add custom acquirers, should you need them.

CONFIGURATION FOR DEVELOPMENT AND TESTING
-----------------------------------------

- By default the merchant/acquirer combination "iDEAL Simulator
  Professional/Advanced/Zelfbouw" will be used. This will make iDEAL API make
  transactions using http://ideal-simulator.nl, a public iDEAL testing server.
- By default all transactions are made using the testing server of whatever
  acquirer is used.

CONFIGURATION FOR PRODUCTION SITES
----------------------------------

- If your bank requires you to perform test transactions, make those using
  iDEAL Donation Form configuring it to use your bank's testing server and by
  entering the test amounts in the donation form manually. Afterwards, check if
  the transaction statuses match the ones that should be returned according to
  your bank's documentation.
- Uncheck "Use testing servers only" at Administer > Configuration > Web
  services > iDEAL API > Global configuration.
- Create your own private key, private key password and public certificate.
  Check the documentation provided by your bank for specific requirements.
  Store the key and certificate in a folder outside your Drupal root that is
  not web-accessible and cannot be read by other users on the same system.
- Store the public certificate of the acquirer you want to use on the server.
- Create a new merchant at Administer > Configuration > Web services > iDEAL
  API and, if you need one, a custom acquirer at Administer > Configuration >
  Web services > iDEAL API > Acquirers.
- Configure cron to run at least every 24 hours, because transaction statuses
  may only be requested within 24 hours of transaction expiration.

TRANSACTION OVERVIEW
--------------------

iDEAL Administration Interface provides an overview of all transactions and
their statuses at Administer > Reports > iDEAL transactions.