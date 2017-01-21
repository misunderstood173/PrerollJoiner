This program works with *.mp4 video files. It will join the selected videos with the specified prerolls (preroll + video + preroll)

Requirements:
ffmpeg.exe in the current folder. You can download it from here: https://ffmpeg.zeranoe.com/builds/
MediaInfo.exe in the current folder. You can download it from here (get the CLI version): https://mediaarea.net/nn/MediaInfo/Download/Windows


In the output folder it will be created a folder with the name of the preroll containing videos joined with that preroll, for each preroll.
In the current folder it will be created a "Temp" folder containing the currently converting video in .ts format. It will be deleted at the end of conversion.
In the current folder it will be created a "Prerolls" folder containing the prerolls in .ts format. These will be kept in case they will be needed on further conversions.

The program will check if the videos and prerolls have 1280x720 resolution.
It won't allow you to select videos with more than 7 words (word separators: space , . : ). The selected videos names will be corrected such as every word will start with uppercase and words will be separated by only one space.
It will check for audio sample rate differences between the videos and prerolls. If it finds any, the preroll will be re-encoded to the video's sample rate. It will also be put in "Prerolls" folder with the same name + frequency. (eg. preroll named "Preroll.mp4" has 44100 Hz audio sample rate and video has 48000, then the preroll will be re-encoded with 48000 audio sample rate, it will be placed in "Prerolls" folder with the name "Preroll 48000.ts"). These will be kept in case they will be needed on further conversions.

Steps:
1. input videos
-check "Videos Folder" and set the videos folder
-check "Select Videos" and browse videos, these will be added at the end of the current list
2. prerolls
-add prerolls using the "Add Preroll" button
3. output folder
-set the output folder using the "Browse" button
4. click the Start button

By default:
-videos folder is: currentFolder/Downloads
-prerolls will be taked from: currentFolder/Prerolls to use
-output folder is: currentFolder/Joined

You can stop the process by clicking the Stop button. It will stop after it finishes the current file conversion. Or you can simply close the program for a forced stop.

If the joined videos are too short or have 0 bytes, make sure you have enough free space on disk. The program will prompt a warning message if you don't have enough free space, but that's an approximation, free up more than it says.
If that isn't the the problem try deleting the "Prerolls" folder and re-convert.