// Copyright 2018, 3D Perception Lab

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "ImageUtils.h"
#include "ROXBasePawn.h"
#include "ROXJsonParser.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "SharedPointer.h"
#include "ROXTracker.generated.h"

/*****************************************************************************
	ENUMERATIONS
*****************************************************************************/

// Lists the different types of images that can be generated with the system.
UENUM(BlueprintType)
enum class EROXViewMode : uint8
{
	RVM_Lit			UMETA(DisplayName = "Lit"),
	RVM_Depth		UMETA(DisplayName = "Depth"),
	RVM_ObjectMask	UMETA(DisplayName = "Object Mask"),
	RVM_Normal		UMETA(DisplayName = "Normal")
};
// Contains the types of images that will be generated for a concrete execution.
static TArray<EROXViewMode> EROXViewModeList;
// First and last elements from EROXViewModeList (updated in runtime)
static EROXViewMode EROXViewMode_First = EROXViewMode::RVM_Lit;
static EROXViewMode EROXViewMode_Last = EROXViewMode::RVM_Normal;


// Lists the different formats available for RGB images.
UENUM(BlueprintType)
enum class EROXRGBImageFormats : uint8
{
	RIF_PNG			UMETA(DisplayName = "PNG"),
	RIF_JPG95		UMETA(DisplayName = "JPG (95%)"),
	RVM_JPG80		UMETA(DisplayName = "JPG (80%)")
};



/*****************************************************************************
	CLASSES
*****************************************************************************/

/*****************************************************************************
* AROXTracker is an omniscient empty actor that should be added to the UE4
* Editor's scene. It will gather all the information needed from the scene to
* rebuild it offline frame per frame, and store that info in JSON format files.
* Then, with each frame being rebuilt independently, four types of ground truth
* images can be generated and stored: RGB, Depth, Object Mask and Normal.
*****************************************************************************/
UCLASS()
class ROBOTRIX_API AROXTracker : public AActor
{
	GENERATED_BODY()

protected:
	/* Record Mode for TRUE, Playback Mode for FALSE */
	UPROPERTY(EditAnywhere)
	bool bRecordMode;

	/* Directory where the raw TXT and JSON files folder will be created */
	UPROPERTY(EditAnywhere)
	FString scene_save_directory;
	/* Folder where raw TXT and JSON files will be stored */
	UPROPERTY(EditAnywhere)
	FString scene_folder;

public:
	/* List of Pawns whose position and pose will be tracked */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Recording)
	TArray<AROXBasePawn*> Pawns;
	/* List of Cameras whose position and rotation will be tracked */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Recording)
	TArray<ACameraActor*> CameraActors;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Name prefix for the raw TXT scene files */
	UPROPERTY(EditAnywhere, Category = Recording)
	FString scene_file_name_prefix;

	/* JSON parser's input: raw TXT scene file name (without extension) */
	UPROPERTY(EditAnywhere, Category = "JSON Management")
	FString input_scene_TXT_file_name;
	/* JSON parser's output: sequence JSON file name (without extension) */
	UPROPERTY(EditAnywhere, Category = "JSON Management")
	FString output_scene_json_file_name;

	/* List of sequences (JSON file names without extension) to be rebuilt */
	UPROPERTY(EditAnywhere, Category = Playback)
	TArray<FString> json_file_names;
	/* List of start rebuild frames for the corresponding sequence from the previous sequence list */
	UPROPERTY(EditAnywhere, Category = Playback)
	TArray<int> start_frames;

	/* If checked, RGB images (JPG RGB 8bit) will be generated for each frame of rebuilt sequences */
	UPROPERTY(EditAnywhere, Category = Playback)
	bool generate_rgb;
	/* Format for RGB images (PNG ~3MB, JPG 95% ~800KB, JPG 80% ~120KB) */
	UPROPERTY(EditAnywhere, Category = Playback)
	EROXRGBImageFormats format_rgb;
	/* If checked, Depth images (PNG Gray 16bit) will be generated for each frame of rebuilt sequences */
	UPROPERTY(EditAnywhere, Category = Playback)
	bool generate_depth;
	/* If checked, Object Mask images (PNG RGB 8bit) will be generated for each frame of rebuilt sequences */
	UPROPERTY(EditAnywhere, Category = Playback)
	bool generate_object_mask;
	/* If checked, Normal images (PNG RGB 8bit) will be generated for each frame of rebuilt sequences */
	UPROPERTY(EditAnywhere, Category = Playback)
	bool generate_normal;
	/* If checked, a TXT file with depth in cm for each pixel will be printed (WARNING: large size ~2MB per file) */
	UPROPERTY(EditAnywhere, Category = Playback)
	bool generate_depth_txt_cm;

	/* Directory where the folder for storing generated images from rebuilt sequences will be created */
	UPROPERTY(EditAnywhere, Category = Playback)
	FString screenshots_save_directory;
	/* Folder where generated images from rebuilt sequences will be stored */
	UPROPERTY(EditAnywhere, Category = Playback)
	FString screenshots_folder;

	/* Width size for generated images */
	UPROPERTY(EditAnywhere, Category = Playback)
	int screenshot_width;
	/* Height size for generated images */
	UPROPERTY(EditAnywhere, Category = Playback)
	int screenshot_height;

	/* Number of frames until the next status output. At the beginning of the execution it will be shown more frequently. */
	UPROPERTY(EditAnywhere, Category = Playback, AdvancedDisplay)
	int frame_status_output_period;
	/* Seconds to wait since execution starts until rebuild process does.*/
	UPROPERTY(EditAnywhere, Category = Playback, AdvancedDisplay)
	float initial_delay;
	/* Seconds to wait since skeletons are placed until cameras can be placed (needed for avoiding failures with bone cameras positions, 0.1 is enough).*/
	UPROPERTY(EditAnywhere, Category = Playback, AdvancedDisplay)
	float place_cameras_delay;
	/* Seconds to wait since rebuild is done until first camera is set and viewmode is changed (0.1 is enough).*/
	UPROPERTY(EditAnywhere, Category = Playback, AdvancedDisplay)
	float first_viewmode_of_frame_delay;
	/* Seconds to wait since last image is generated until next viewmode can be changed (0.1 is enough, 0.2 is also good for slower PCs).*/
	UPROPERTY(EditAnywhere, Category = Playback, AdvancedDisplay)
	float change_viewmode_delay;
	/* Seconds to wait since viewmode has changed until image is generated (0.1 is enough).*/
	UPROPERTY(EditAnywhere, Category = Playback, AdvancedDisplay)
	float take_screenshot_delay;

	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bStandaloneMode;
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	FString Persistence_Level_Filter_Str;
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bDebugMode;

	bool bIsRecording;

	uint64 numFrame;
	int64 JsonReadStartTime;
	int64 LastFrameTime;

	// Cached properties
	TArray<AStaticMeshActor*> CachedSM;
	TArray<bool> CachedSM_Gravity;
	TArray<bool> CachedSM_Physics;

	UMaterial* DepthMat;
	UMaterial* DepthWUMat;
	UMaterial* DepthCmMat;
	UMaterial* NormalMat;
	ASceneCapture2D* SceneCapture_depth;
	UTextureRenderTarget2D* DepthTextureRenderer;

	TArray<AActor*> ViewTargets;
	int CurrentViewTarget;
	int CurrentCamRebuildMode;
	EROXViewMode CurrentViewmode;
	int CurrentJsonFile;

	AROXBasePawn* ControllerPawn;

	ROXJsonParser* JsonParser;
	FROXFrame currentFrame;

private:
	/* Complete name for raw TXT files */
	FString absolute_file_path;

	/* Flag for avoiding write header several times when appending to the same file. By default appends will not happen. */
	bool fileHeaderWritten;

public:
	// Sets default values for this actor's properties
	AROXTracker();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void WriteHeader();
	void WriteScene();
	void PrintInstanceClassJson();
	void ToggleRecording();

	FString GetDateTimeString();

	APostProcessVolume* GetPostProcessVolume();
	void SetVisibility(FEngineShowFlags& Target, FEngineShowFlags& Source);
	void VertexColor(FEngineShowFlags& ShowFlags);
	void PostProcess(FEngineShowFlags& ShowFlags);
	FEngineShowFlags* GameShowFlags;

	void Lit();
	void Object();
	void Depth();
	void Normal();
	void HighResSshot(UGameViewportClient* ViewportClient, const FString& FullFilename, const EROXViewMode viewmode);
	AActor* CameraNext();
	AActor* CameraPrev();
	void TakeScreenshot(EROXViewMode vm = EROXViewMode::RVM_Lit);
	void TakeScreenshotFolder(EROXViewMode vm, FString CameraName);
	void TakeDepthScreenshotFolder(const FString& FullFilename);
	void ChangeViewmode(EROXViewMode vm);
	FString ViewmodeString(EROXViewMode vm);
	EROXViewMode NextViewmode(EROXViewMode vm);
	void CacheStaticMeshActors();
	void ChangeViewmodeDelegate(EROXViewMode vm);
	void TakeScreenshotDelegate(EROXViewMode vm);

	void CacheSceneActors(const TArray<FString> &PawnNames, const TArray<FString> &CameraNames);
	void DisableGravity();
	void RestoreGravity();
	void RebuildModeBegin();
	void RebuildModeMain();
	void RebuildModeMain_Camera();
	void PrintStatusToLog(int startFrame, int64 startTimeSec, int64 lastFrameTimeSec, int currentFrame, int64 currentTimeSec, int totalFrames);

	UFUNCTION(CallInEditor, BlueprintCallable, Category="JSON Management")
	void GenerateSequenceJson();

	FORCEINLINE bool GetMode() const
	{
		return bRecordMode;
	}

	FORCEINLINE bool isDebugMode() const
	{
		return bDebugMode;
	}
};


/*
**
*/
class FWriteStringTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FWriteStringTask>;

public:
	FWriteStringTask(FString str, FString absoluteFilePath) :
		m_str(str),
		m_absolute_file_path(absoluteFilePath)
	{}

protected:
	FString m_str;
	FString m_absolute_file_path;

	void DoWork()
	{
		// Place the Async Code here.  This function runs automatically.
		// Text File
		FFileHelper::SaveStringToFile(m_str, *m_absolute_file_path, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}

	// This next section of code needs to be here.  Not important as to why.

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FWriteStringTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};


/*
**
*/
class FScreenshotTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FScreenshotTask>;

public:
	FScreenshotTask(TArray<uint8> bitarray, FString absoluteFilePath) :
		m_bitarray(bitarray),
		m_absolute_file_path(absoluteFilePath)
	{}

protected:
	TArray<uint8> m_bitarray;
	FString m_absolute_file_path;

	void DoWork()
	{
		// Text File
		FFileHelper::SaveArrayToFile(m_bitarray, *m_absolute_file_path);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FScreenshotTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};