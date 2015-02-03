/*
 * Rijksoverheid Cookie OPT-IN v1.0
 * http://www.rijksoverheid.nl/cookies/
 * 
 * Copyright 2012 Rijksoverheid.nl, 
 * Public Information and Communication Office, 
 * Ministry of General Affairs
 * 
 * This script is distributed under the 
 * Creative Commons attribution-share alike license:
 * http://creativecommons.org/licenses/by-sa/3.0/
 * 
 */ 


/* Sample
 *
 */
$(document).ready(function()
{
  // First, set up cookies interface.
  $.rocookies.init();

  // Start cookiebar with a set of options.
  $.rocookiebar.init(
  {
    'NL-NL': // Dutch version for xml:lang 'nl-nl'
    {
      'question': // Base texts for question screen
      {
        'title': 'Cookies op Rijksoverheid.nl',
        'intro': 'Mag Rijksoverheid.nl <a href="/cookies/">cookies</a> op uw computer plaatsen om de site prettiger in het gebruik te maken?',
        'learn-more-links': '<p>Ik wil eerst meer weten ...</p><ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li></ul>'
      },
      'change': // Base texts for change of preference
      {
        'title': 'Uw cookievoorkeur veranderen op Rijksoverheid.nl',
        'intro': 'Mag Rijksoverheid.nl <a href="/cookies/">cookies</a> op uw computer plaatsen om de site prettiger in het gebruik te maken?',
        'learn-more-links': '<p>Ik wil eerst meer weten ...</p><ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li></ul>'
      },
      'accept': // All texts for 'agree' option: button, explanations, thank-you text, etc.
      {
        'button': 'Ja, ik accepteer deze cookies',
        'extras': '<ul><li>Rijksoverheid.nl verzamelt anonieme bezoekgegevens ter verbetering van de website.</li></ul>',
        'title': 'U accepteert cookies van Rijksoverheid.nl',
        'intro': 'Dank voor uw toestemming aan Rijksoverheid.nl om cookies op uw computer te plaatsen. U kunt altijd uw <a href="#" class="ck-change">toestemming veranderen</a>.',
        'current': 'U accepteert cookies van Rijksoverheid.nl, maar kunt dit hieronder veranderen.',
        'learn-more-links': '<ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li></ul>'
      },
      'deny': // All texts for 'disagree' option: button, explanations, thank-you text, etc.
      {
        'button': 'Nee, ik accepteer deze cookies niet',
        'extras': '<ul><li>Rijksoverheid.nl verzamelt geen gegevens over uw bezoek via cookies.</li></ul>',
        'title': 'Rijksoverheid.nl respecteert uw keuze',
        'intro': 'Deze website gebruikt geen cookies om gegevens over uw bezoek te verzamelen.',
        'current': 'U accepteert nu geen cookies van Rijksoverheid.nl en kunt dit hieronder veranderen.',
        'learn-more-links': '<ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li><li><a href="#" class="ck-change">Cookievoorkeur veranderen</a></li></ul>'
      },
      'close': '<a href="#main">Deze balk sluiten</a>' // Close link on thank you screens
    },

    'EN-GB':  // English version for xml:lang 'en-gb'
    {
      'question':
      {
        'title': 'Cookies on Mywebsite.nl',
        'intro': 'May Mywebsite.nl place <a href="/cookies/">cookies</a> on your computer to make the site easier for you to use?',
        'learn-more-links': '<p>I first want to know more ...</p><ul><li><a href="/cookies/">What do cookies do?</a></li><li><a href="/privacy/">What about my privacy?</a></li></ul>'
      },
      'change':
      {
        'title': 'Changing your cookie preferences on Mywebsite.nl',
        'intro': '',
        'learn-more-links': '<p>I first want to know more ...</p><ul><li><a href="/cookies/">What do cookies do?</a></li><li><a href="/privacy/">What about my privacy?</a></li></ul>'
      },
      'accept':
      {
        'button': 'Yes, I accept cookies',
        'extras': '<ul><li>Mywebsite.nl will collect anonymous information about your visits to help improve the website.</li></ul>',
        'title': 'You have chosen to accept cookies from Mywebsite.nl',
        'intro': 'Thank you for permitting Mywebsite.nl to place cookies on your computer. You can always <a href="#" class="ck-change">change your mind</a>.',
        'current': 'You now accept cookies from Mywebsite.nl. You can change your preference below. May Mywebsite.nl continue to place <a href="/cookies/">cookies</a> on your computer to make the site easier for you to use?',
        'learn-more-links': '<ul><li><a href="/cookies/">What do cookies do?</a></li><li><a href="/privacy/">What about my privacy?</a></li></ul>'
      },
      'deny':
      {
        'button': 'No, I don&rsquo;t accept cookies',
        'extras': '<ul><li>Mywebsite.nl will not use cookies to collect anonymous information about your visits.</li></ul>',
        'title': 'Mywebsite.nl respects your choice',
        'intro': 'This website will not use cookies to collect information about your visits.',
        'current': 'You do not accept cookies from Mywebsite.nl. You can change this preference below. May Mywebsite.nl place cookies on your computer to make the site easier for you to use?',
        'learn-more-links': '<ul><li><a href="/cookies/">What do cookies do?</a></li><li><a href="/privacy/">What about my privacy?</a></li><li><a href="#" class="ck-change">Change your cookie preferences</a></li></ul>'
      },
      'close': '<a href="#main">Close this menu bar</a>'
    },

    'cookiename': 'toestemmingvoorcookies',
    'cookievalues':
    {
      'accept': 'ja',
      'deny': 'nee'
    },

    'cookieurl': 'presentation/images/cookie.png',  // This option is a custom setting used in the callback.

    'callback': function(result)
    {
      // If visitor agreed, we need to document this by setting a cookie and logging an URL with a unique code for legal reasons.
      // If the visitor disagreed, we just set a 'deny' cookie and request the image (cookieless!) without a unique code.
      var   agent = navigator.userAgent.hashCode(),
            now = new Date(),
            timestamp = Date.UTC(now.getFullYear(), now.getMonth(), now.getDate(), now.getHours(), now.getMinutes(), now.getSeconds(), now.getMilliseconds()),
            uniqueid = timestamp + agent.toString(),
            lifespan = $.rocookiebar.settings['lifespan'] || 5*365,
            consent = $.rocookiebar.settings['cookievalues'][result],
            cookielog = new Image();

      if (result == "accept")
      {
        consent = consent + "." + uniqueid;
        
        // Add statistics code here, e.g. google analytics startup, comscore, or others.
        // This code is only triggered once, on the page where the visitor agrees to accept cookies.
      }

      // Remember choice in cookie.
      $.rocookies.create($.rocookiebar.settings['cookiename'],consent,$.rocookiebar.settings['lifespan']);

      // Fetch an image to log visitor choice on server, with a unique code if visitor agreed.
      cookielog.src = $.rocookiebar.settings['cookieurl'] + "?" + $.rocookiebar.settings['cookiename'] + "=" + consent;
    }
  });
  
  
  // Open change preference screen if visitor clicks first newslink on sample page.
  // This is to demonstrate how the opt-in change screen can be opened from within a webpage.
  $('#leaderboard .list-common li:first').click(function(evt)
  {
    evt.preventDefault();
    $.rocookiebar.build('change');
  });


  
  // If a visitor returns after making a choice, you can test for the cookie as follows.
  var ck = $.rocookies[$.rocookiebar.settings['cookiename']] || '';
  if (ck.indexOf($.rocookiebar.settings['cookievalues']['accept']) > -1)
  {
    // Put your cookie sensitive code, like statistics, here.
    alert("Bezoeker heeft cookies geaccepteerd. Yay!");
  }
});