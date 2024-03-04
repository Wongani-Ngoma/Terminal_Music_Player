#include <conio.h>
#include <filesystem>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <time.h>
#include <vector>

#include <SFML/Audio.hpp>
#include <SFML/Main.hpp>

#include <fstream>

namespace fs = std::filesystem;

enum LookUpMode {
    files_only = 0,
    files_and_folders = 1,
};

//How songs in the music list are played
enum PlayMode {
    play_sequentially = 0,
    play_single_song = 1,
    play_randomly = 2,
};

//Functions
std::string getFormatedDuration(float duration);
bool loadSongsInCurrentFolderAndSubfolders(std::string& thePath);
bool loadSongsInCurrentFolderExcludingSubfolders(std::string& thePath);
bool playMusic(std::string& filepath);
bool loadMusicToQueueSequentially();
bool loadMusicToQueueInRandomOrder();
bool loadSingleSongToQueue();
bool playMusicInPlayingQueue();
bool playMusicInPlayingQueue(int pos);
void updateInput();
void initilizeSettings();
void setPlayMode(PlayMode newPlayMode);
void printToLog();

//Globals
std::vector<std::string> musicList;
std::vector<std::string> playingQueue;
sf::Music currentMusic; //Instance of the music that is currently being played
std::string currentMusicStr; //Directory of the music that is currently being played
int currentMusicIndex = 0; //Index of the music in playing queue that is currently being played

std::string folderToLookForMusic = "C:\\Users\\Ngoma\\Music";
LookUpMode lookUpMode;
PlayMode playMode;
bool repeat;
bool shouldStop; //To tell the program whether to continue playing the playingQueue or not

int main() {

    int result;
    std::cout << "Enter folder path\n: ";
    std::getline(std::cin, folderToLookForMusic);

    initilizeSettings();

    //Loading music in music list
    if (lookUpMode == LookUpMode::files_and_folders) {
        result = loadSongsInCurrentFolderAndSubfolders(folderToLookForMusic);
    }
    else {
        result = loadSongsInCurrentFolderExcludingSubfolders(folderToLookForMusic);
    }
    // Return if theres no music in the folder
    if (result == false) {
        std::cout << "No music files found in " << folderToLookForMusic << "\n";
        return 0;
    }

    // Loading music from music list into playing queue
    if (playMode == PlayMode::play_sequentially) {
        loadMusicToQueueSequentially();
    }
    else if(playMode == PlayMode::play_randomly) {
        loadMusicToQueueInRandomOrder();
    }
    else { //Play single song
        loadSingleSongToQueue();
    }

    std::cout << "\nHere are the files\n\n";
    for (int i = 0; i < playingQueue.size(); i++) {
        std::cout << i + 1 << " - " << playingQueue[i] << "\n";
    }
    int songIndexToStartPlayingFrom;
    do {
        std::cout << "\nWhich song to start playing from(Enter the song's assigned number): ";
        std::cin >> songIndexToStartPlayingFrom;
    } while (songIndexToStartPlayingFrom < 1 || songIndexToStartPlayingFrom > playingQueue.size());
    songIndexToStartPlayingFrom--; //Make it into an index

    //Play music in the playing queue
    do {
        playMusicInPlayingQueue(songIndexToStartPlayingFrom);
        songIndexToStartPlayingFrom = 0; //This means from now onwards in this loop, music will be played from the start
    } while (repeat == true);
        
    return 0;
}

void initilizeSettings() {
    int response;
    shouldStop = false;
    std::cout << "Here are the files in " << folderToLookForMusic << "\n\n";

    std::cout << "Enter look up mode\n\t1. Files only\n\t2. Files and folders\n\t : ";
    std::cin >> response;
    lookUpMode = (response == 1 ? LookUpMode::files_only : LookUpMode::files_and_folders);

    std::cout << "Enter play mode\n\t1. Play sequentially\n\t2. Play randomly\n\t3. Play single song\n\t :";
    std::cin >> response;
    if (response == 1) {
        playMode = PlayMode::play_sequentially;
    }
    else if (response == 2) {
        playMode = PlayMode::play_randomly;
    }
    else {
        playMode = PlayMode::play_single_song;
    }

    std::cout << "Enable repeat? ('1' for yes, '2' for no)\n\t :";
    std::cin >> response;
    repeat = (response == 1 ? true : false);
}
std::string getFormatedDuration(float durationInSec) {
    std::string formatedDuration;
    int hours = durationInSec / 3600;
    int minutes = (durationInSec / 60) - hours * 60;
    int seconds = durationInSec - (minutes * 60);

    formatedDuration = std::to_string(hours) + ":";
    formatedDuration += std::to_string(minutes) + ":";
    formatedDuration += std::to_string(seconds);

    return formatedDuration;

}

//Analyses 'thePath' and pushes the full directory of each song in 'thePath' and it's folders and their sub-folders etc
bool loadSongsInCurrentFolderAndSubfolders(std::string& thePath) {
    static bool foundAtLeastOneSong = false;
    std::string outfilestr;
    for (const auto& entry : fs::directory_iterator(thePath)) {
        fs::path outfilename = entry.path();
        try {
            outfilestr = outfilename.string();
            if (fs::is_directory(outfilestr)) {
                foundAtLeastOneSong = loadSongsInCurrentFolderAndSubfolders(outfilestr);
            }
            else {
                if (outfilestr.substr(outfilestr.length() - 3, 3) == "mp3") {
                    musicList.push_back(outfilestr);
                    foundAtLeastOneSong = true;

                }
            }
        }
        catch (std::system_error e) {
            //Do nothing
        }

    }

    return foundAtLeastOneSong;

}

//Analyses 'thePath' and pushes the full directory of each song in the folder 'thePath', sub folders are not considered
bool loadSongsInCurrentFolderExcludingSubfolders(std::string& thePath) {
    static bool foundAtLeastOneSong = false;
    std::string outfilestr;
    for (const auto& entry : fs::directory_iterator(thePath)) {
        fs::path outfilename = entry.path();
        try {
            outfilestr = outfilename.string();
            if (fs::is_regular_file(outfilename)) { //Check if it's a file, not a folder
                if (outfilestr.substr(outfilestr.length() - 3, 3) == "mp3") { //Check if it's an .mp3 file
                    musicList.push_back(outfilestr);
                    foundAtLeastOneSong = true;

                }
            }
        }
        catch (std::system_error e) { //This exeption occurs when outfilename refered to a file with a non-ascii character
            //For now we do nothing
        }

    }

    return foundAtLeastOneSong;

}

void updateInput() {
    if (_kbhit()) {
        char pressedKey = _getch();
        pressedKey = std::tolower(pressedKey);
        switch (pressedKey) {
        case 's':
            shouldStop = true;
            break;
        case 'n':
            if (currentMusicIndex < playingQueue.size()) {
                currentMusic.stop();
            }
            break;
        case ' ':
            if (currentMusic.getStatus() == sf::Music::Playing) {
                currentMusic.pause();
            }
            else {
                currentMusic.play();
            }
            break;
        case 'p':
            if (currentMusicIndex > 0) {
                currentMusic.stop();
                currentMusicIndex -= 2;
            }
            else {
                currentMusic.stop();
                currentMusicIndex = playingQueue.size() - 2;
            }
            break;
        case 'r': //Set play mode to random
            printToLog();
            setPlayMode(PlayMode::play_randomly);
            printToLog();
            break;
        case 'l': //Toogle repeat on/off
            repeat = !repeat;
            break;
        default:
            break;
        }
    }
}

bool playMusic(std::string& filepath) {

    if (!currentMusic.openFromFile(filepath)) {
        return false;
    }

    std::cout << "Duration: " << getFormatedDuration(currentMusic.getDuration().asSeconds()) << "\n";
    std::cout << "Number of channels: " << currentMusic.getChannelCount() << "\n";
    std::cout << "Sample rate: " << currentMusic.getSampleRate() << "\n";

    currentMusic.play();

    std::cout << "\rPlaying: " << filepath << std::endl;
    while (shouldStop == false) {
        //Leave some CPU time for other running processes
        sf::sleep(sf::milliseconds(100.0f));

        std::cout << "\r  - " << getFormatedDuration(currentMusic.getPlayingOffset().asSeconds());
        std::cout << std::flush;
        updateInput();

    }
    std::cout << std::endl << std::endl;
    return true;
}

bool loadMusicToQueueSequentially() {
    for (std::string& str : musicList) {
        playingQueue.push_back(str);
    }

    return true;
}
bool loadMusicToQueueInRandomOrder() {

    srand(time(0));
    std::list<std::string> temp;
    int randIndex;


    for (std::string& str : musicList) {
        temp.push_back(str);
    }

    auto tempIter = temp.begin();

    while (temp.size() > 0) {
        randIndex = rand() % temp.size();
        std::advance(tempIter, randIndex);
        playingQueue.push_back(*tempIter);

        temp.remove(playingQueue.back());
        tempIter = temp.begin();
    
    }

    return true;

}

bool loadSingleSongToQueue() {
    playingQueue.push_back(musicList[0]);
    return true;
}

bool playMusicInPlayingQueue() {
    while (currentMusicIndex < playingQueue.size()) {
        currentMusicStr = playingQueue[currentMusicIndex];
        if (playMusic(playingQueue[currentMusicIndex]) == false) {
            return false;
        }
        currentMusicIndex++;
    }
    return true;
}

bool playMusicInPlayingQueue(int pos) {
    currentMusicIndex = pos;
    while (currentMusicIndex < playingQueue.size()) {
        currentMusicStr = playingQueue[currentMusicIndex];
        if (playMusic(playingQueue[currentMusicIndex]) == false) {
            return false;
        }
        currentMusicIndex++;
    }
    return true;
}

//Here we set the new play mode and re-structure the playing queue based on the new play mode
void setPlayMode(PlayMode newPlayMode) {
    playMode = newPlayMode;
    playingQueue.clear();
    std::vector<std::string>::iterator iter;

    if (newPlayMode == PlayMode::play_randomly) {
        loadMusicToQueueInRandomOrder();
        iter = playingQueue.begin();
        while (*iter != currentMusicStr && iter != playingQueue.end()) {
            iter++;
        }
        if (iter != playingQueue.end()) playingQueue.erase(iter);
        currentMusicIndex = 1;
    } else if (newPlayMode == PlayMode::play_sequentially) {
        iter = playingQueue.begin();
        loadMusicToQueueSequentially();
        currentMusicIndex = 1;
    } else if (newPlayMode == PlayMode::play_single_song) {
        //Nothing, what needs to be done will be done at this function's end ----
    }//                                                                         | Here
    playingQueue.insert(playingQueue.begin(), currentMusicStr);//   <-----------|

}

//For debuging purposes
void printToLog() {
    std::ofstream file("log.txt", std::ios::app);
    if (file.is_open()) {
        file << "----------------------------\n------- Start of Playing queue ----------\n----------------------------";
        for (std::string str : playingQueue) {
            file << str << "\n";
        }
        file << "----------------------------\n------- End of Playing queue ----------\n----------------------------";
        file << "                                                                                                   ";
        file.close();
    }
    else {
        std::cout << "Failed to open log file\n";
    }
}