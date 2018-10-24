"use strict";

var graph = (function() {
   let ctx, oscillator;

   let now = function() {
      return ctx.currentTime;
   };

   let prepareHtmlElements = function() {
      var controllersDiv = document.querySelector('#controllers');

      var startAudioButton = document.createElement ('button');
      startAudioButton.appendChild (document.createTextNode('startAudio'));
      startAudioButton.onclick = function() {
         startAudio();
      }

      startAudioButton.style.background = "#FFFFFF";
      controllersDiv.appendChild (startAudioButton);

      var stopButton = document.createElement ('button');
      stopButton.appendChild (document.createTextNode('stop'));
      stopButton.onclick = function() {
         stop();
      }

      stopButton.style.background = "#FFFFFF";
      controllersDiv.appendChild (stopButton);
   }

   window.addEventListener ('load', prepareHtmlElements);

   let startAudio = function() {
      var AudioContext = window.AudioContext || window.webkitAudioContext;

      // Context declaration (default context)
      ctx = new AudioContext();

      oscillator = new OscillatorNode(ctx, {
         type: "sine",
         frequency: 440,
         detune: 0
      });

      oscillator.connect(ctx.destination);

      oscillator.start();
   };

   let stop = function() {
      oscillator.stop();
   }

})();