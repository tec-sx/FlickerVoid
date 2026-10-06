#pragma once

#include "CoreMinimal.h"

class AFVMapCaptureActor;
class UTexture2D;

/** Renders map capture actors and stores the result in their map definitions. */
namespace FVMapCapture
{
	struct FResult
	{
		bool bSuccess = false;
		UTexture2D* Texture = nullptr;
		FText Message;
	};

	/**
	 * Renders the actor's box from above, saves the image to a texture (updating the layer's existing texture
	 * when it has one) and writes the texture and captured area into the actor's map layer.
	 */
	FResult Capture(AFVMapCaptureActor& Actor);

	/** Capture with an editor notification showing the result. */
	FResult CaptureAndNotify(AFVMapCaptureActor* Actor);

	/** Capture actors placed in the level open in the editor. */
	TArray<AFVMapCaptureActor*> FindCaptureActors();
}
