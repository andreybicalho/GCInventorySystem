// Copyright (c) 2024 Gamecan OÜ.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GCInventoryFunctionLibrary.generated.h"

class AActor;
class UGCActorInventoryComponent;

/**
 * 
 */
UCLASS()
class GCINVENTORYSYSTEM_API UGCInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Gamecan|Inventory", meta = (WorldContext = "WorldContextObject"))
	static UGCActorInventoryComponent* GetInventoryComponentFromActor(const UObject* WorldContextObject, const AActor* actor, const bool bLookForComponent = true);
};
