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

(function($)
{
  // Add hash method to string object
  String.prototype.hashCode = function()
  {
    var ha = 0, i, ch, l = this.length;
    if (l == 0) return ha;
    for (i = 0; i < l; i++) {
      ch = this.charCodeAt(i);
      ha = ((ha<<5)-ha)+ch;
      ha = ha & ha; // Convert to 32bit integer
    }
    return ha;
  };
  
  // Cookies library
  $.rocookies = 
  {
    supported: false,
    
    init: function (options) {
      var pos, cookieName, cookieValue, allCookies;

      for (var i in this) {
        if (typeof this[i] == 'function') continue;
        this[i] = undefined;
      }
  
      allCookies = document.cookie.split('; ');
      for (var i = 0, l = allCookies.length; i < l; i++) {
        pos = allCookies[i].indexOf('=');
        if (pos != -1) {
          cookieName = allCookies[i].substr(0, pos);
          cookieValue = allCookies[i].substr(pos + 1, allCookies[i].length)
          this[cookieName] = cookieValue;
        }
      }
      
      // Try a test cookie to check support
      this.create('dough','baking',1);
      if (this.read('dough') !== null) {
        this.supported = true;
        this.erase('dough');
      }
    },
    create: function (name,value,days) {
      var dmn = '', ckdmn = '', date, expires;
      if (days) {
        date = new Date();
        date.setTime(date.getTime()+(days*24*60*60*1000));
        expires = "; expires="+date.toGMTString();
      }
      else
      {
        expires = "";
      }
      
      ckdmn = window.location.hostname.split(".");
      dmn = "; domain=" + ckdmn[ckdmn.length-2] + "." + ckdmn[ckdmn.length-1];
  
      document.cookie = name+"="+value+expires+"; path=/" + dmn;
      this[name] = value;
    },
    read: function (name) {
      var nameEQ = name + "=",
    	    ca = document.cookie.split(';'),
    	    c;
    	for (var i = 0, j = ca.length; i < j; i++) {
    		c = ca[i];
    		while (c.charAt(0)==' ') c = c.substring(1,c.length);
    		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    	}
    	return null;    
    },
    erase: function (name) {
      this.create(name,'',-1);
      this[name] = undefined;
    },
    eraseAll: function () {
      for (var i in this) {
        if (typeof this[i] == 'function') continue;
        this.erase(i);
      }
    }
  };

  $.rocookiebar =
  {
    settings:
    {
      'cookiename': 'toestemmingvoorcookies',
      'cookievalues':
      {
        'accept': 'ja',
        'deny': 'nee'
      },
                  
      'top': 0,
      
      'lifespan': 5*365,
      
      'callback': function(result) { $.rocookies.create($.rocookiebar.settings['cookiename'],$.rocookiebar.settings['cookievalues'][result],$.rocookiebar.settings['lifespan']); },
      
      'langCode': '',
      
      'NL-NL':
      {
        'question':
        {
          'title': 'Cookies op Mijnwebsite.nl',
          'intro': 'Mag Mijnwebsite.nl <a href="/cookies/">cookies</a> op uw computer plaatsen om de site prettiger in het gebruik te maken?',
          'learn-more-links': '<p>Ik wil eerst meer weten ...</p><ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li></ul>'
        },
        'change':
        {
          'title': 'Uw cookievoorkeur veranderen op Mijnwebsite.nl',
          'intro': 'Mag Mijnwebsite.nl <a href="/cookies/">cookies</a> op uw computer plaatsen om de site prettiger in het gebruik te maken?',
          'learn-more-links': '<p>Ik wil eerst meer weten ...</p><ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li></ul>'
        },
        'accept':
        {
          'button': 'Ja, ik accepteer deze cookies',
          'extras': '<ul><li>U kunt handige extra’s op de site gebruiken, zoals Google-kaarten of de Twitter-knop.</li><li>Mijnwebsite.nl verzamelt anonieme bezoekgegevens ter verbetering van de site.</li></ul>',
          'title': 'U accepteert cookies van Mijnwebsite.nl',
          'intro': 'Dank voor uw toestemming aan Mijnwebsite.nl om cookies op uw computer te plaatsen. U kunt altijd uw <a href="#" class="ck-change">toestemming veranderen</a>.',
          'current': 'U accepteert cookies van Mijnwebsite.nl, maar kunt dit hieronder veranderen.',
          'learn-more-links': '<ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li></ul>'
        },
        'deny':
        {
          'button': 'Nee, ik accepteer deze cookies niet',
          'extras': '<ul><li>U kunt handige extra’s, zoals Google-kaarten en de Twitter-knop, niet gebruiken.</li><li>Mijnwebsite.nl verzamelt geen gegevens over uw bezoek via cookies.</li></ul>',
          'title': 'Mijnwebsite.nl respecteert uw keuze',
          'intro': 'Deze website gebruikt geen cookies om gegevens over uw bezoek te verzamelen. Sommige handige extra’s werken alleen met cookies, bijvoorbeeld Google-kaarten en de Twitter-knop. Deze extra’s kunt u dus op deze site niet gebruiken.',
          'current': 'U accepteert nu geen cookies van Mijnwebsite.nl en kunt dit hieronder veranderen.',
          'learn-more-links': '<ul><li><a href="/cookies/">Wat doen cookies?</a></li><li><a href="/privacy/">Hoe zit het met mijn privacy?</a></li><li><a href="#" class="ck-change">Cookievoorkeur veranderen</a></li></ul>'
        },
        'close': '<a href="#main">Deze balk sluiten</a>'
      }
    },
    
    langCode: '',

    htmlsnippet: '' +
        '<div class="ck-center">' +
          '<div class="ck-main">' +
            '<div class="ck-content">' + 
              '<p id="ck-attention"></p>' +
              '<p id="ck-intro"></p>' +
              '<div id="ck-options"></div>' +
              '<div id="ck-links"></div>' +
            '</div>' +
          '</div>' +
          '<div class="ck-footer"></div>' +
        '</div>',
    
    buttonsnippet: '' +
        '<div class="ck-accept">' +
          '<a href="#ck-attention"></a>' +
        '</div>' +
        '<div class="ck-deny">' +
          '<a href="#ck-attention"></a>' +
        '</div>',
    

    // START build function
    build: function(situ)
    {
      if (typeof situ == 'undefined') situ = 'question';
      
      var bar = $("#cookiebar"), newbar = false, origpad, cookieroom, consent,
          touch = ('ontouchstart' in document.documentElement),
          iphone = touch && (navigator.userAgent.indexOf("iPhone") > -1),
          ios6 = touch && (navigator.userAgent.indexOf("OS 6_0") > -1);
      
      if (bar.size() < 1)
      {
        bar = $("<div/>").attr('id','cookiebar').html(this.htmlsnippet);
        newbar = true;
      }

      // Fill out all texts within snippet
      bar.removeClass('answered').find(".ck-center").removeClass('hover');
      bar.find("#ck-attention").html(this.settings[this.langCode][situ]['title']);
      if (situ == 'question')
      {
        bar.find("#ck-intro").html(this.settings[this.langCode][situ]['intro']);
      }
      else
      {
        var ck = $.rocookies[$.rocookiebar.settings['cookiename']] || '';
        consent = (ck.indexOf($.rocookiebar.settings['cookievalues']['accept']) > -1)
                ? 'accept'
                : 'deny';
        bar.find("#ck-intro").html(this.settings[this.langCode][consent]['current'] + " " + this.settings[this.langCode][situ]['intro']);
      }
      bar.find("#ck-options").html(this.buttonsnippet);
      bar.find(".ck-accept a").html(this.settings[this.langCode]['accept']['button']).after(this.settings[this.langCode]['accept']['extras']);
      bar.find(".ck-deny a").html(this.settings[this.langCode]['deny']['button']).after(this.settings[this.langCode]['deny']['extras']);
      bar.find("#ck-links").html(this.settings[this.langCode][situ]['learn-more-links']);
      
      if (touch) bar.addClass("touch");
      if (iphone) bar.addClass("iphone");
      if (ios6) bar.addClass("ios6");
      
      if (newbar)
      {
        bar.css(
        {
          'opacity': 0,
          'top': this.settings['top']
        });

        var origpad = parseInt($("body").css('padding-top'),10);
        $("body").attr('data-orig-padding',origpad);
        
        if (touch)
        {
          bar.css(
          {
            'position': 'absolute'
          });
        }
      }

      // Add cookiebar to DOM, as the first element.
      $("body").prepend(bar);

      // Prepare foldout and other events.
      bar.find("ul").each(function()
      {
        var $this = $(this); $this.attr('data-orig-height',$this.height()).css(
        {
          'height': 0,
          'overflow': 'hidden'
        });
      });

      bar.find(".ck-center").mouseenter(function()
      {
        $.rocookiebar.expand(this);
      }).mouseleave(function()
      {
        $.rocookiebar.collapse(this);
      });      
      
      if (touch)
      {
        $("#main-box").click(function()
        {
          var cbar = $(".ck-center");
          if (cbar.hasClass("hover"))
          {
            $.rocookiebar.collapse(cbar);
          }
        });
      }
      
      
      bar.find("#ck-options a, #ck-links a").focus(function()
      {
        $.rocookiebar.expand(".ck-center");
      }).blur(function()
      {
        $.rocookiebar.collapse(".ck-center");
      });

      bar.find("#ck-options a").click(function(evt)
      {
        var q = ($(this).parent().hasClass("ck-accept")) ? 'accept' : 'deny';

        evt.preventDefault();
        evt.stopPropagation();
        // Change UI, set cookie and trigger callback
        $.rocookiebar.thankyou(q);
        $.rocookiebar.settings.callback(q);
      });

      bar.find("#ck-options div").mouseenter(function()
      {
        $(this).addClass("option-hover");
      }).mouseleave(function()
      {
        $(this).removeClass("option-hover");
      }).click(function()
      {
        var q = ($(this).hasClass("ck-accept")) ? 'accept' : 'deny';

        // Change UI, set cookie and trigger callback
        $.rocookiebar.thankyou(q);
        $.rocookiebar.settings.callback(q);
      });

      // Make room for the cookiebar by animating the body padding.
      // This appears to be the most robust solution given the variety of front-end architectures this should work in.
      if (newbar)
      {
        cookieroom = origpad + bar.height() + this.settings['top'];
        $("body").animate(
        {
          'padding-top': cookieroom
        },1200,function()
        {
          bar.animate({ 'opacity': 1 },1000);
        });
      }
    },
    // END build function


    // START expand function
    expand: function(elem)
    {
      $(elem).addClass("hover").find("ul").each(function()
      {
        var $this = $(this), h = $this.attr('data-orig-height');
        $this.stop().animate(
        {
          'height': h
        },400);
      });      
    },
    // END expand function
    

    // START collapse function
    collapse: function(elem)
    {
      $(elem).removeClass("hover").find("ul").each(function()
      {
        $(this).stop().animate(
        {
          'height': 0
        },400);
      });
    },
    // END collapse function


    // START alter function
    thankyou: function(situ)
    {
      if (typeof situ == 'undefined' || situ == '') return;
      var cb = $("#cookiebar").addClass("answered");
      cb.find("#ck-attention").html(this.settings[this.langCode][situ]['title']);
      cb.find("#ck-intro").html(this.settings[this.langCode][situ]['intro']);
      cb.find("#ck-links").html(this.settings[this.langCode][situ]['learn-more-links']);
      cb.find("#ck-options").html(this.settings[this.langCode]['close']);
      cb.find("#ck-options a").click(function(evt)
      {
        evt.preventDefault();
        $.rocookiebar.hide();
      });
      
      cb.find("a.ck-change").click(function(evt)
      {
        evt.preventDefault();
        $.rocookiebar.build('change');
      });
      
      cb.find(".ck-center").unbind('mouseenter, mouseleave');
    },
    // END alter function
    

    // START hide function
    hide: function()
    {
      var cb = $("#cookiebar"), b = $("body"), p = parseInt(b.attr('data-orig-padding'),10);
      cb.animate({ 'opacity': 0 }, 600, function(){ $(this).remove(); });
      b.animate({ 'padding-top': p }, 800);
    },
    // END hide function
  

    init: function(options)
    {
      // If browser 'don't track' is enabled, skip
      if (typeof window.navigator.doNotTrack !== 'undefined' && window.navigator.doNotTrack == "yes") return;
  
      // If blocking class on body is present, skip.
      if ($("body").hasClass('nocookiebar')) return;

      this.settings = $.extend(this.settings, options);
    
      langCode = $("html").attr('xml:lang');
      this.langCode = langCode.toUpperCase();
      if (typeof this.settings['langCode'] == 'string' && this.settings['langCode'] !== '')
      {
        this.langCode = this.settings['langCode'];
      }
      
      if (typeof this.settings[this.langCode] === 'undefined') return; // Chosen language set is not provided, so quit.
      
      // If cookies are not supported, or visitor already chose, also skip.
      if (!$.rocookies.supported) return;

      if (typeof $.rocookies[$.rocookiebar.settings['cookiename']] !== 'undefined') return;
  
      // From here, we have a go, so build a cookie jar.
      this.build();
    }
  }
})(jQuery);