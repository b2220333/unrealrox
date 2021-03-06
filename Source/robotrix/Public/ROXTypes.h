// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROXTypes.generated.h"

USTRUCT()
struct FROXActorState
{
	GENERATED_USTRUCT_BODY()

	FVector Position;
	FRotator Rotation;

	FROXActorState()
	{}
};

USTRUCT()
struct FROXActorStateExtended
{
	GENERATED_USTRUCT_BODY()

	FVector Position;
	FRotator Rotation;
	FVector BoundingBox_Min;
	FVector BoundingBox_Max;

	FROXActorStateExtended()
	{}
};

USTRUCT()
struct FROXSkeletonState
{
	GENERATED_USTRUCT_BODY()

	FVector Position;
	FRotator Rotation;
	TMap<FString, FROXActorState> Bones;

	FROXSkeletonState()
	{}
};

USTRUCT()
struct FROXFrame
{
	GENERATED_USTRUCT_BODY()

	int n_frame;
	float time_stamp;
	TMap<FString, FROXActorState> Cameras;
	TMap<FString, FROXActorStateExtended> Objects;
	TMap<FString, FROXSkeletonState> Skeletons;

	FROXFrame()
	{}
};