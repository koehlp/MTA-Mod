# MTA-Mod

Parts of the readme and code are from https://github.com/fabbrimatteo/JTA-Mods/

# MTA-Mod (beta)
This repository contains two Grand Theft Auto V Mods which were used for creating the MTA Dataset (https://github.com/schuar-iosb/mta-dataset) presented in the paper LINK
- FunctionToolbox: Contains functions which help to set the camera views e.g. or to explore GTA V.
- DatasetCreator: To create a multi target multi camera tracking dataset. 

## Quick start
- Download and install a legal copy of Grand Theft Auto V
- Download ScriptHook V AND the SDK [here](http://www.dev-c.com/gtav/scripthookv/). 
    - Follow the instructions reported on the web page. Also, follow the instructions of the SDK readme under the section "Runtime asi script reloading"
	- Basically create empty file called "ScriptHookV.dev" in the GTA V game folder C:\Program Files\Rockstar Games\Grand Theft Auto V for asi reloading
- Download the FunctionToolbox Mod: [`FunctionToolbox.asi`] https://drive.google.com/uc?export=download&id=1m35nRR9P1I9ZI-NGqKFGid3vAYAGIkrk
- Download the DatasetCreator Mod: [`DatasetCreator.asi`] https://drive.google.com/uc?export=download&id=1CtbKw-hJip7hFLdMS9a3bvp_V3jMKafp
- Download the MTMCT Config files : [`MTMCT.zip`] https://drive.google.com/uc?export=download&id=1H5tQdVZJYgzhWJce7hNe2UjXEzagD3HY
- Copy both files `FunctionToolbox.asi`, `DatasetCreator.asi` and the MTMCT folder into the Grand Theft Auto V game folder.
- If you installed GTA V using Rockstar's Social Club, you have to change the permissions of the game folder (read, write and execute). If you used Steam, everything should be fine.
- Run Grand Theft Auto V and select Story Mode. If it is your first start of the game, you have to complete the first mission. Enjoy!
- Activate in GTA V Windowed Borderless: Settings -> Graphics -> Screen Type -> Windowed Borderless. Otherwise black images might be recorded.
- The mods will automatically start. Now, by pressing F5 you can use the FunctionToolbox mod. By pressing F6 the DatasetCreator mod menu will open. You can disable the Mods in any time by pressing ctrl+R.
- Navigate via num pad arrows in the menus. You can start recording by choosing `Start combined recording` and pressing Num 5. Wait a few seconds.
- Hold F10 to stop the recording. The recorded images and annotations will be in the GTA V game C:\Program Files\Rockstar Games\Grand Theft Auto V\MTMCT\cam_0 etc.
- Remember to disable the HUD in the display settings of GTA V. Also disable the in-game notifications.





## Developer Guide
- Download and install a legal copy of Grand Theft Auto V
- Clone this repository
- Download ScriptHook V AND the SDK here http://www.dev-c.com/gtav/scripthookv/. Follow the instructions reported on the web page. Also, follow the instructions of the SDK readme under the section "Runtime asi script reloading"
- Basically create empty file called "ScriptHookV.dev" in the GTA V game folder C:\Program Files\Rockstar Games\Grand Theft Auto V for asi reloading
- Copy the ScriptHook V SDK folders inc and lib into the repository folder into MTA-Mod/deps/
- Open the Solution with Visual Studio (we used VS2017, but previous versions should work too) and, for each sub-project, follow these instructions:
	- include the header files to the project by going in Configuration Properties->C/C++->General and set the path to the "inc" directory of the SDK in "Additional Include Directories"
	- include the library to the projects by going to Configuration Properties->Linker->General  and set the path that points to the "lib" folder of the SDK in "Additional Library Directories" 
	- Also go to:  `Configuration Properties -> Linker -> Input`  and set the `ScriptHookV.lib` file name in `Additional Dependencies`
	- Go in `Build-Events -> Post-Build-Event` and, under `Command Line` type `xcopy /Y path/to/asi/file path/to/gta/installation/directory`
	- Under `Configuration-Properties -> General` change the `Target Extension` to `.asi` and `Configuration Type` to `.dll`
- Build the entire solution.
- If you installed GTA V using Rockstar's Social Club, you have to change the permissions of the game folder (read, write and execute). If you used Steam, everything should be fine.
- Run Grand Theft Auto V and select Story Mode. If it is your first start of the game, you have to complete the first mission. Enjoy!
- The mods will automatically start. Now, by pressing F5 you can use the FunctionToolbox mod. By pressing F6 the DatasetCreator mod menu will open. You can disable the Mods in any time by pressing ctrl+R.
- Remember to disable the HUD in the display settings of GTA V. Also disable the in-game notifications.
- If you want clear undistorted recordings you have to install this mod  https://de.gta5-mods.com/misc/no-chromatic-aberration-lens-distortion-1-41


## FunctionToolbox usage

- Start this mod by pressing F5
- Fly around to take a desired camera view with the player by activating Fly mode. Move up by pressing E and down by Q.
- Press F6 to open the DatasetCreator and add a new camera view. This view will be appended to MTMCT/cameraSettings.csv


## DatasetCreator usage

- Start this mod by pressing F6
- Start the recording by choosing `Start combined recording`.
- Stop the recording by holding F10.


## Citation

We believe in open research and we are happy if you find this code useful.   
If you use it, please cite our work.

The affiliated paper was published at the CVPR 2020 VUHCS Workshop (https://vuhcs.github.io/)

```latex
@InProceedings{Kohl_2020_CVPR_Workshops,
    author = {Kohl, Philipp and Specker, Andreas and Schumann, Arne and Beyerer, Jurgen},
    title = {The MTA Dataset for Multi-Target Multi-Camera Pedestrian Tracking by Weighted Distance Aggregation},
    booktitle = {The IEEE/CVF Conference on Computer Vision and Pattern Recognition (CVPR) Workshops},
    month = {June},
    year = {2020}
}
```




