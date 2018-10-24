"use strict";

var graph = (function() {
   let ctx, mediaElementAudioSource, reverbSend, convolver, master, ir, loop;

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
   }

   let prepareSliders = function() {
      // Quick way of checking if this code was already executed...
      if (document.getElementById ("reverbSendSlider") != undefined)
          return;
      
      // Helper method to create HTML range input and a text label.
      function createSlider (id, label, min, max, defaultValue, inputCb) {
          let slider = document.createElement("input");
          slider.id = id;
          slider.setAttribute("type", "range");
          slider.setAttribute("min", min);
          slider.setAttribute("max", max);
          slider.value = defaultValue;
          slider.style.width = "100%";
          slider.oninput = inputCb;
      
          var pElement = document.createElement("p");
          pElement.appendChild(document.createTextNode(label));
          document.getElementById("controllers").appendChild(pElement);
          document.getElementById("controllers").appendChild(slider);
      
          return slider;
      }
      
      // This is a concise way to add a bunch of sliders with custom callback methods
      let rvbSlider = createSlider ("reverbSendSlider", "Reverb send :", 0, 100, 50, function() {
          reverbSend.gain.setValueAtTime(rvbSlider.value / 100, now());
      });
      
      let gainSlider = createSlider ("masterSlider", "Master gain :", 0, 100, 50, function() {
          master.gain.setValueAtTime(gainSlider.value / 100, now());
      });
   };

   let loadAudio = function() {
      let adCtx = new AudioContext();

      let predecoder = new AudioPredecoder();

      ir = new DecodableAudio (predecoder, adCtx, 0, "data/ir.wav", 1);

      loop = document.createElement('audio');
      loop.autoplay = 0;
      loop.controls = 1;
      loop.loop = 1;
      loop.muted = 0;
      loop.preload = "auto";
      loop.src = "data/loop.mp3";
      document.querySelector("#controllers").appendChild (loop);

      predecoder.onSuccess = prepareHtmlElements;
      predecoder.predecode();
   }

   window.addEventListener ('load', loadAudio);

   let startAudio = function() {
      var AudioContext = window.AudioContext || window.webkitAudioContext;

      // Context declaration (default context)
      ctx = new AudioContext();

      mediaElementAudioSource = new MediaElementAudioSourceNode(ctx, {
         mediaElement: loop
      });

      reverbSend = new GainNode(ctx, {
         gain: 0.5
      });

      convolver = new ConvolverNode(ctx, {
         buffer: ir.buffer,
         disableNormalization: false
      });

      master = new GainNode(ctx, {
         gain: 0.5
      });

      convolver.connect(master);
      reverbSend.connect(convolver);
      mediaElementAudioSource.connect(reverbSend);
      mediaElementAudioSource.connect(master);
      master.connect(ctx.destination);

      prepareSliders();
   };

})();