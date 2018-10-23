# WebAudio Visual Editor

WebAudio Visual Editor is an open-source project that aims at giving the HTML5 WebAudio API a powerful GUI, in the form of a desktop software written in C++ with JUCE. 

Here are a few design goals :
* Visual editing : audio graphs can be edited in a modular view. Other elements can be added to the graph such as dynamic containers, messages to nodes, comments or JS sub-scripts. The resulting JS code is generated accordingly as well as a HTML test project.
* Re-usability : graph elements can be saved as presets. This makes it easy to share the most common portions of code between projects and prepare templates for complex tasks.
* Designed as a playground : the most basic actions must be intuitive to the user, even with a poor knowledge in digital audio techniques. Working project examples are included and testing a project in the browser is as simple as hitting a button! If only a few things can be achieved without typing Javascript code, the software aims at providing the user with suggestions and make easier tedious tasks such as loading an audio file.

Please note that for now, the software is more suited to prototyping than to 'serious' web deployment. Most notably, the software is based on the [WebAudio API w3c spec](https://www.w3.org/TR/webaudio/) which is not fully implemented in all browsers. You can have more info on supported platforms on [Mozilla's guide](https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API).

## Getting Started

To automatically generate project files for your integrated developement environment, download the [latest version of JUCE](https://shop.juce.com/get-juce) the  and open "_WebAudio Visual Editor.jucer_" with Projucer. For more information, here is a [complete tutorial](https://docs.juce.com/master/tutorial_new_projucer_project.html).

### Prerequisites

You'll need the library code from JUCE. You can get the latest release [here](https://shop.juce.com/get-juce) or clone the library code from [GitHub](https://github.com/WeAreROLI/JUCE).

## Built With

* [JUCE](https://juce.com/) - Cross-platform C++ library
* [Mongoose module for JUCE](https://github.com/cpenny42/juce_mongoose) - A JUCE wrapper for [Mongoose](https://github.com/cesanta/mongoose) embedded web server

## Contributing

Feel free to submit contributions. For code styling, please refer to [JUCE's coding standards](https://juce.com/discover/stories/coding-standards).

## Authors

* **Pierre-Clément Kerneïs** - *Initial work* - [pckerneis](https://github.com/pckerneis)

See also the list of [contributors](https://github.com/pckerneis/WebAudioVisualEditor/contributors) who participated in this project.

## License

This project is licensed under the GPL v3 License - see the [LICENSE.md](LICENSE.md) file for details
