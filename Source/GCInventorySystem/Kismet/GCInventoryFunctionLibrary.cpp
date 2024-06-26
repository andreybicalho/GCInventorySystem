// Copyright (c) 2024 Gamecan OÜ.


#include "GCInventoryFunctionLibrary.h"
#include "Interfaces/GCInventoryInterface.h"
#include "Components/GCActorInventoryComponent.h"

UGCActorInventoryComponent* UGCInventoryFunctionLibrary::GetInventoryComponentFromActor(const UObject* WorldContextObject, const AActor* actor, const bool bLookForComponent /* = true */)
{
	if (actor == nullptr)
	{
		return nullptr;
	}

	if (const IGCInventoryInterface* inventoryInterface = Cast<IGCInventoryInterface>(actor))
	{
		return inventoryInterface->GetInventoryComponent();
	}

	if (bLookForComponent)
	{
		// Fall back to a component search to better support BP-only actors
		return actor->FindComponentByClass<UGCActorInventoryComponent>();
	}

	return nullptr;
}
