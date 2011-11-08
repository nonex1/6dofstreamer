// Socket.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "utils.h"
#include "SocketConnector.h"


using namespace std;

CRITICAL_SECTION criticalSection;
Head* head;

void createSocket( void* parameter ){
	printf("In thread\n");
	SocketConnector socketConnector(criticalSection, head);
	
}


//************************ faceapi *****************

// Save an image to PNG file
smReturnCode saveToPNGFile(const std::string& filepath, smImageInfo image_info)
{
    smBool ok;
    smReturnCode error;
    
    // Create an API string
    smStringHandle filepath_handle = 0;
    ok = (error = smStringCreate(&filepath_handle)) == SM_API_OK;
    ok = ok && (error = smStringReadBuffer(filepath_handle,filepath.c_str(),filepath.size())) == SM_API_OK;

    // Create an API image
    smImageHandle image_handle = 0;       
    smImageMemoryCopyMode copy_mode = SM_API_IMAGE_MEMORYCOPYMODE_AUTO;        
    ok = ok && (error = smImageCreateFromInfo(&image_info,&copy_mode,&image_handle)) == SM_API_OK;

    // Save the image as PNG
    ok = ok && (error = smImageSaveToPNG(image_handle,filepath_handle)) == SM_API_OK;

    // Destroy the image and string
    smStringDestroy(&filepath_handle);
    smImageDestroy(&image_handle);
    return error;
}

// Handles head-tracker face-data callbacks
void STDCALL receiveFaceData(void *, smEngineFaceData face_data, smCameraVideoFrame video_frame)
{   
    static int num_face_textures = 0;
    // If face_data contains a texture, save it to disk as a PNG file.
    if (face_data.texture)
    {       
        // Create a unique filename
        std::stringstream filename;
        filename << "my_face_" << ++num_face_textures << ".png";        
        // Try saving
        if (saveToPNGFile(filename.str(), face_data.texture->image_info) == SM_API_OK)
        {
            cout << "Saved face-texture to " << filename.str() << endl;
        }
        else
        {
            cout << "Error saving face-texture to " << filename.str() << endl;
        }
    }
}

// Handles head-tracker head-pose callbacks
void STDCALL receiveHeadPose(void *,smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame)
{
	if(&head_pose!=NULL){
    // Make output readable
    fixed(cout);
    showpos(cout);
    cout.precision(2);

    cout << "Head Pose: ";
    cout << "pos(";
    cout << head_pose.head_pos.x << ",";
    cout << head_pose.head_pos.y << ",";
    cout << head_pose.head_pos.z << ") ";
    cout << "rot(";
    cout << rad2deg(head_pose.head_rot.x_rads) << ",";
    cout << rad2deg(head_pose.head_rot.y_rads) << ",";
    cout << rad2deg(head_pose.head_rot.z_rads) << ") ";
    cout << "conf " << head_pose.confidence;
    cout << endl;

	EnterCriticalSection(&criticalSection);
		head->x  = head_pose.head_pos.x;
		head->y  = head_pose.head_pos.y;
		head->z  = head_pose.head_pos.z;
		head->rx = rad2deg(head_pose.head_rot.x_rads);
		head->ry = rad2deg(head_pose.head_rot.y_rads);
		head->rz = rad2deg(head_pose.head_rot.z_rads);	
		head->confidence  = head_pose.confidence; // by Murivan on 07/11/2011 to export also confidence value
	LeaveCriticalSection(&criticalSection);
	}

}

// Create the first available camera detected on the system, and return its handle
smCameraHandle createFirstCamera()
{
    // Detect cameras
    smCameraInfoList info_list;
    THROW_ON_ERROR(smCameraCreateInfoList(&info_list));

    if (info_list.num_cameras == 0)
    {
        throw std::runtime_error("No cameras were detected");
    }
    else
    {
        cout << "The followings cameras were detected: " << endl;
        for (int i=0; i<info_list.num_cameras; ++i)
        {
            char buf[1024];
            cout << "    " << i << ". Type: " << info_list.info[i].type;
            THROW_ON_ERROR(smStringWriteBuffer(info_list.info[i].model,buf,1024));
            cout << " Model: " << std::string(buf);
            cout << " Instance: " << info_list.info[i].instance_index << endl;
            // Print all the possible formats for the camera
            for (int j=0; j<info_list.info[i].num_formats; j++)
            {
                smCameraVideoFormat video_format = info_list.info[i].formats[j];
                cout << "     - Format: ";
                cout << " res (" << video_format.res.w << "," << video_format.res.h << ")";
                cout << " image code " << video_format.format;
                cout << " framerate " << video_format.framerate << "(hz)";
                cout << " upside-down? " << (video_format.is_upside_down ? "y":"n") << endl;
            }
        }
    }

    // Create the first camera detected on the system
    smCameraHandle camera_handle = 0;
    THROW_ON_ERROR(smCameraCreate(&info_list.info[0], 
                                  0 /* Don't override any settings */, 
                                  &camera_handle));

    // Destroy the info list
    smCameraDestroyInfoList(&info_list);

    return camera_handle;
}

// The main function
void run()
{
	try
  {


    // Get the version
    int major, minor, maint;
    THROW_ON_ERROR(smAPIVersion(&major, &minor, &maint));
    cout << endl << "API VERSION: " << major << "." << minor << "." << maint << "." << endl << endl;

    // Register the WDM category of cameras
    THROW_ON_ERROR(smCameraRegisterType(SM_API_CAMERA_TYPE_WDM));

    const bool non_commercial_license = smAPINonCommercialLicense() == SM_API_TRUE;

    smEngineHandle engine_handle = 0;
    smCameraHandle camera_handle = 0;
    if (non_commercial_license)
    {
        cout << "Non-Commercial License restrictions apply, see doco for details." << endl;

        // Create a new Head-Tracker engine that uses the camera
        THROW_ON_ERROR(smEngineCreate(SM_API_ENGINE_LATEST_HEAD_TRACKER,&engine_handle));
    }
    else
    {
        // Print out a list of connected cameras, and choose the first camera on the system
        camera_handle = createFirstCamera();

        // Create a new Head-Tracker engine that uses the camera
        THROW_ON_ERROR(smEngineCreateWithCamera(SM_API_ENGINE_LATEST_HEAD_TRACKER,camera_handle,&engine_handle));
    }

    // Check license for particular engine version (always ok for non-commercial license)
    const bool engine_licensed = smEngineIsLicensed(engine_handle) == SM_API_OK;

    // Hook up callbacks to receive output data from engine.
    // This function will return an error if the engine is not licensed.
    if (engine_licensed)
    {
        THROW_ON_ERROR(smHTRegisterHeadPoseCallback(engine_handle,0,receiveHeadPose));
        if (!non_commercial_license)
        {
            THROW_ON_ERROR(smHTRegisterFaceDataCallback(engine_handle,0,receiveFaceData));
        }
    }
    else
    {
        cout << "Engine is not licensed, cannot obtain any output data." << endl;
    }

    if (!non_commercial_license)
    {
        // Enable lip and eyebrow tracking    
        THROW_ON_ERROR(smHTSetLipTrackingEnabled(engine_handle,SM_API_TRUE));
        THROW_ON_ERROR(smHTSetEyebrowTrackingEnabled(engine_handle,SM_API_TRUE));
    }

    // Create and show a video-display window
	smVideoDisplayHandle video_display_handle = 0;
	THROW_ON_ERROR(smVideoDisplayCreate(engine_handle,&video_display_handle,0,TRUE));

    // Get the handle to the window and change the title to "Hello World"
    smWindowHandle win_handle = 0;
    THROW_ON_ERROR(smVideoDisplayGetWindowHandle(video_display_handle,&win_handle));    
    SetWindowText(win_handle, _T("FaceAPI Non-Commercial Version"));

    // Setup the VideoDisplay so we only see the face mask overlay
    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,SM_API_VIDEO_DISPLAY_HEAD_MESH));

    // Start tracking
	THROW_ON_ERROR(smEngineStart(engine_handle));

    // Perform tracking until a key is hit.
    while (processKeyPress(engine_handle,win_handle))
    {
        // NOTE: If you have a windows event loop in your program you 
        // will not need to call smAPIProcessEvents(). This manually redraws the video window.
        THROW_ON_ERROR(smAPIProcessEvents());
        // Prevent CPU hog in our simple loop.
        Sleep(10); 
    }

    // Destroy engine
    THROW_ON_ERROR(smEngineDestroy(&engine_handle));
    // Destroy video display
    THROW_ON_ERROR(smVideoDisplayDestroy(&video_display_handle));

	  }
  catch (...)
  {
    cout << "An exception occurred. Exception Nr. " << endl;
	exit(0);
  }


} // run()

void read6dof(void* parameter){
	printf("Reading face values\n");
	try
    {
#       ifdef _DEBUG
        // Log API debugging information to a file
        THROW_ON_ERROR(smLoggingSetFileOutputEnable(SM_API_TRUE));
        // Hook up message logging when debugging
        THROW_ON_ERROR(smLoggingRegisterCallback(0,(smLoggingCallback)receiveLogMessage));
#       endif

    	// Initialize the API
	    THROW_ON_ERROR(smAPIInit());
        run();
    }
    catch (std::exception &e)
    {
        cerr << e.what() << endl;
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
	try{
		InitializeCriticalSection(&criticalSection);
		head = new Head();
		head->x = 0.5f;
		head->y = 40.0f;
		head->z = -40.0f;
		
		_beginthread(createSocket,0,NULL);
		//_beginthread(read6dof,0,NULL);

		int count = 0;

		read6dof(NULL);
		/*while(1){
				Sleep(1000);*/

		/*	float headx;
			scanf("%f",&headx);*/
			
			//count++;
			//if(count % 10000 == 0){
				//Sleep(15);
				//head->x = headx;
				//EnterCriticalSection(&criticalSection);
					
				//LeaveCriticalSection(&criticalSection);
			
			//}

		//}
	}
	catch(...){
		cout << "Unhandled exception" << endl;
	}

    return 0;
}



