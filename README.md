# Xfeed
Xfeed is audio processing software for improving the experience of listening to stereo music on headphones through crossfeed. It is available as a 64-bit cross-platform VST3 plugin, or as a standalone 64-bit program for Windows 10 and above. 

[![Latest release][release-img]][release-url]

## Installation
There are multiple ways of using Xfeed. Dowload the VST3 plugin and/or Windows executable from the assets folder of the [latest release](https://github.com/abhinavnatarajan/Xfeed/releases). 
1. To use the VST3 plugin, copy `Xfeed.vst3` into your VST3 folder.

    * On Windows this is usually `C:\Program Files\Common Files\VST3`.
    * On Mac OS there are two folders for VST3 plugins, one for the current user `~/Library/Audio/Plug-Ins/VST3`, and a systemwide folder `Macintosh HD/Library/Audio/Plug-Ins/VST3`.

   The VST3 plugin is meant primarily for use inside a digital audio workstation, but can also be used for all system audio via any lightweight VST host program that is capable of running VST3 plugins. More instructions are provided below. 

2. To use the standalone version on Windows, Xfeed must be able to access system audio. To do this, install software such as [VB-Cable](https://vb-audio.com/Cable/index.htm) or [JACK](https://jackaudio.org/) and use them to set up a virtual audio input/audio output pair. Use the virtual output as your system audio playback device. Then simply place `Xfeed.exe` anywhere you like and run it. Use the options menu in Xfeed to choose the virtual audio input as the audio source, and set your headphone device as the audio output. 

3. Rolling your own: you could also compile Xfeed yourself. There are a few reasons why you may want to do this:

    * You would like to build a standalone version for Mac OS. The source code for Xfeed includes the necessary files to create a standalone version on Mac. At the time of writing, I merely don't have a Mac to compile and build this standalone version.
    * You would like to build AAX, RTAS, or AU versions of Xfeed for use in Pro Tools. 

   Xfeed was built using the JUCE framework (v7.0.2) in C++ and compiled with MSVC C++20. The simplest way to generate the makefiles necessary to compile Xfeed is to use the program `Projucer.exe` (which is included with the JUCE library) to open `Xfeed.jucer` and generate the project structure. 

## Usage
There are only three settings in Xfeed:
1. A bypass switch which disables crossfeed. Xfeed smoothly interpolates between processed and unprocessed audio when this switch is toggled to prevent audio artifacts such as pops and crackles from occurring.
1. A gain knob, which you can use to compensate for volume changes when Xfeed is bypassed. 
1. An angle knob that controls the angle from which the crossfeed is originates in 3D space. This parameter only affects the crossfeed signal and not the dry signal, and will therefore not create a "binaural effect" when it is changed. 

## Recommendations
1. Xfeed should be placed at the very end of your signal chain. Make sure to disable any system audio enhancements like Dolby Spatial Audio or Windows Sonic for headphones. 
2. Crossfeed naturally improves bass perception. This is true to life; stereo speakers improve bass perception due to the natural crossfeed effect and due to the acoustic shadow of your head and torso. In most cases this is desirable, but you might wish to apply a low shelf cut of 1 or 2 dB at around 1000Hz to compensate for the increased bass if you prefer. 
3. The JUCE library also includes an audio plugin host that you can use in the same way as the standalone version of Xfeed, but it must be compiled from source. The process for compiling and building the plugin host is straightforward and well-documented. The JUCE plugin host allows for more flexibility than the standalone version of Xfeed, since you can use the host to chain together Xfeed with other plugins such as headphone frequency correction impulse responses and/or room reverbs to simulate a realistic home or studio listening environment. 


## FAQs
**Q.** What is crossfeed?

**A.** Crossfeed refers to how our ears typically perceive sounds from both the left and right audio channels when listening to audio played on stereo speakers. Headphones do not usually have any crossfeed beacause the left and right channels of audio are isolated the left and right ear. This results in an unnaturally wide and unrealistic soundstage. 


**Q.** Does crossfeed work by mixing the left and right audio channels together? 

**A.** The very simplest of crossfeed algorithms do exactly this, but Xfeed goes further. Our ears use the tiny differences between the same sound arriving at the left and right ears to determine the origin of that sound in 3D space. These differences include the time at which that sound arrives (inter-aural time difference, or ITD) and differences in the frequency response (both magnitude and phase) between the sounds arriving at the left and right ears. In particular, your head casts an acoustic 'shadow' between your left and right ears, so sound that travels from one side to the other experiences changes that aren't captured by just the time difference. 


**Q.** Is crossfeed the same thing as binaural audio?

**A.** No, although there are several similarities. The main goal of crossfeed is to improve spatial localisation in the left-right plane of the soundstage when listening on headphones, particularly when mixing music, and to make listening to music on headphones less fatiguing. The goal of binaural audio is to recreate a highly realistic 3D soundstage using only stereo audio. This gets very complicated very quickly, since factors like the shape of your head, the shape of your earlobes, and the size and shape of your torso affect the way you perceive sound. Binaural audio is often achieved through the use of Head Related Impulse Responses (HRIRs), which are filters that simulate these various factors, but the downside of using generic HRIRs is that they perceptibly colour the sound in erratic ways. This is fine for casual listening such as gaming and movie watching, but HRIRs fall short for mixing music. The only real solution is to use individualised HRIRs, but these are difficult to compute. I am not aware of any commercial services that currently offer individualised HRIR measurement (as of 2022).


**Q.** Are there other crossfeed plugins?

**A.** Yes. [CanOpener Studio](https://goodhertz.com/canopener-studio/) by GoodHertz Inc and [Nx Virtual Mix Room](https://www.waves.com/plugins/nx#introducing-nx-virtual-mix-room) by Waves Audio are two paid alternatives. Both plugins offer many more features than Xfeed, but both plugins are paid software and are not open source. Waves Nx goes the route of trying to accurately simulate 3D psychoacoustics, and suffers from many of the problems oulined above. See [here](https://goodhertz.com/tonal/canopener-vs-nx/) for an in-depth comparison between CanOpener Studio and Nx Virtual Mix Room. 

[//]: # (Add donation link)

[release-img]: https://img.shields.io/github/v/release/abhinavnatarajan/Xfeed?display_name=tag&logo=SemVer&sort=semver
[release-url]: https://github.com/abhinavnatarajan/RedClust.jl/releases

