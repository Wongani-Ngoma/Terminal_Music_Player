# Terminal_Music_Player
Plays music from the terminal
I used the SFML library to play the audio files, the rest of the code just handles the features you normally find in all music players (shuffle, repeat, selecting folders to look for music from etc).

## Workings
When the program runs, it asks for the full directory to the folder where the music will be searched.
Then it asks for the look up mode, one look up mode only searches for audio files in the directory provided earier and ignores all sub-folders in that folder and the audio files in them, the other look up mode searches for audio files in the directory provided earier as well as the folders in that directory, and their folders.
Then it asks for the play mode, you have 3 choices sequentially, randomly or just one single song.
Then it asks if you want to enable repeat mode, which plays all songs in the playing queue in a loop.
Then it loads the full path name of all songs in the directory provided into musicList, the songs it will load will depend on the look up mode.
Then it load the full path names in musicList into playingQueue, the way the path names are loaded depend on the play mode.
Then it plays the songs in playingQueue in order.
Some info regarding the audio file is displayed on the terminal;

You can enter the following commands while a song is playing:
Press N to play the next song
Press S to stop
Press P to play previous song
Press SPACE to pause/resume the song

### To-do
More command will be added to this terminal version
Planning to add a graphical user interface
