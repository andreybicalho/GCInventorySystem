// Fill out your copyright notice in the Description page of Project Settings.

#include "GCActorInventoryComponent.h"
#include "Interfaces/GCInventoryInterface.h"
#include "Subsystems/GCInventoryGISSubsystems.h"
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY(LogGCActorInventoryComponent);

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCActorInventoryComponent)

UGCActorInventoryComponent::UGCActorInventoryComponent(const FObjectInitializer& ObjectInitializer)
{
	HeldItemTags = FGCGameplayTagStackContainer();
	StartUpItems.Empty();

	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UGCActorInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (StartUpItems.Num() > 0)
	{
		for (const auto& currentItem : StartUpItems)
		{
			AddItemToInventory(currentItem.Key, currentItem.Value);
		}
	}
}

void UGCActorInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, HeldItemTags);
}

bool UGCActorInventoryComponent::AddItemToInventory(FGameplayTag itemTag, float itemStack)
{
	const auto ownerActor = GetOwner();

	if (ownerActor && ownerActor->GetClass()->ImplementsInterface(UGCInventoryInterface::StaticClass()))
	{
		HeldItemTags.AddStack(itemTag, itemStack);

		IGCInventoryInterface::Execute_ItemGranted(ownerActor, itemTag, itemStack);

		OnItemGranted.Broadcast(itemTag, itemStack, ownerActor);

		return true;
	}

	return false;
}

void UGCActorInventoryComponent::UseItemFromInventory(FGameplayTag itemTag, float itemStack)
{
	const auto ownerActor = GetOwner();

	if (ownerActor && ownerActor->GetClass()->ImplementsInterface(UGCInventoryInterface::StaticClass()) && IsItemInInventory(itemTag))
	{
		IGCInventoryInterface::Execute_ItemUsed(ownerActor, itemTag, itemStack);

		OnItemUsed.Broadcast(itemTag, itemStack, ownerActor);
	}
}

void UGCActorInventoryComponent::DropItemFromInventory(FGameplayTag itemTag, float itemStack)
{
	const auto ownerActor = GetOwner();

	if (ownerActor && ownerActor->GetClass()->ImplementsInterface(UGCInventoryInterface::StaticClass()) && ContainsItemInInventory(itemTag, itemStack))
	{
		HeldItemTags.RemoveStack(itemTag, itemStack);

		IGCInventoryInterface::Execute_ItemDropped(ownerActor, itemTag, itemStack);

		OnItemRemoved.Broadcast(itemTag, itemStack, ownerActor);
	}
}

void UGCActorInventoryComponent::DropAllItemsFromInventory()
{
	const auto& heldItems = HeldItemTags.GetGameplayTagStackList();

	if (heldItems.Num() > 0)
	{
		for (const auto& itemStack : heldItems)
		{
			DropItemFromInventory(itemStack.GetGameplayTag(), itemStack.GetStackCount());
		}
	}
}

void UGCActorInventoryComponent::RemoveItemFromInventory(FGameplayTag itemTag, float itemStack)
{
	const auto ownerActor = GetOwner();

	if (ownerActor && ownerActor->GetClass()->ImplementsInterface(UGCInventoryInterface::StaticClass()) && IsItemInInventory(itemTag))
	{
		HeldItemTags.RemoveStack(itemTag, itemStack);

		IGCInventoryInterface::Execute_ItemRemoved(ownerActor, itemTag, itemStack);

		OnItemRemoved.Broadcast(itemTag, itemStack, ownerActor);
	}
}

void UGCActorInventoryComponent::RemoveAllItemsFromInventory()
{
	const auto& heldItems = HeldItemTags.GetGameplayTagStackList();

	if (heldItems.Num() > 0)
	{
		for (const auto& itemStack : heldItems)
		{
			RemoveItemFromInventory(itemStack.GetGameplayTag(), itemStack.GetStackCount());
		}
	}
}

void UGCActorInventoryComponent::ClearInventory()
{
	HeldItemTags.ClearStack();
}

bool UGCActorInventoryComponent::IsItemInInventory(FGameplayTag itemTag) const
{
	return HeldItemTags.ContainsTag(itemTag);
}

bool UGCActorInventoryComponent::ContainsItemInInventory(const FGameplayTag& itemTag, const float amount /*= 1.f*/) const
{	
	return HeldItemTags.ContainsTag(itemTag) && HeldItemTags.GetStackCount(itemTag) >= amount;
}

float UGCActorInventoryComponent::GetItemStack(FGameplayTag itemTag) const
{
	return HeldItemTags.GetStackCount(itemTag);
}

TMap<FGameplayTag, float> UGCActorInventoryComponent::GetAllItemsOnInventory() const
{
	TMap<FGameplayTag, float> currentItemsMap;

	const auto& currentItemsArray = HeldItemTags.GetGameplayTagStackList();

	if (currentItemsArray.Num() > 0)
	{
		for (const auto& itemStack : currentItemsArray)
		{
			currentItemsMap.Add(itemStack.GetGameplayTag(), itemStack.GetStackCount());
		}
	}

	return currentItemsMap;
}

float UGCActorInventoryComponent::GetTotalAmountItems() const
{
	float total = 0.0f;

	const auto& items = HeldItemTags.GetGameplayTagStackList();

	for (const auto& itemStack : items)
	{
		total += itemStack.GetStackCount();
	}

	return total;
}

bool UGCActorInventoryComponent::CraftItem(FGameplayTag itemTag)
{
	const auto ownerActor = GetOwner();

	if (auto inventorySubsystem = UGCInventoryGISSubsystems::Get(ownerActor))
	{
		const auto itemRecipe = inventorySubsystem->GetItemRecipe(itemTag);

		if (IsItemCraftable(itemRecipe))
		{
			for (const auto& recipeElement : itemRecipe.RecipeElements)
			{
				RemoveItemFromInventory(recipeElement.Key, recipeElement.Value);
			}

			if (AddItemToInventory(itemTag, itemRecipe.CraftedQuantity))
			{
				IGCInventoryInterface::Execute_ItemCrafted(ownerActor, itemTag, itemRecipe.CraftedQuantity);

				return true;
			}			
		}
	}

	return false;
}

bool UGCActorInventoryComponent::CanItemBeCrafted(FGameplayTag itemTag)
{
	const auto ownerActor = GetOwner();

	if (auto inventorySubsystem = UGCInventoryGISSubsystems::Get(ownerActor))
	{
		const auto itemRecipe = inventorySubsystem->GetItemRecipe(itemTag);

		return IsItemCraftable(itemRecipe);
	}

	return false;
}

void UGCActorInventoryComponent::BindEventToItemUpdated(const FGameplayTag itemTag, const UObject* delegateOwner, const FDynamicOnStackItemReplicated& eventDelegate)
{
	HeldItemTags.BindDelegateToStackReplicated(itemTag, eventDelegate, delegateOwner);
}

void UGCActorInventoryComponent::BindEventToItemTagStackUpdated(FOnTagStackUpdatedDynamicDelegate eventDelegate)
{
	HeldItemTags.BindDelegateToOnTagStackUpdated(eventDelegate);
}

bool UGCActorInventoryComponent::IsItemCraftable(FItemRecipeElements recipe)
{
	bool bHasMaterials = true;

	if (recipe.RecipeElements.Num() > 0)
	{
		for (const auto& recipeElement : recipe.RecipeElements)
		{
			if (GetItemStack(recipeElement.Key) < recipeElement.Value)
			{
				bHasMaterials = false;
				break;
			}
		}
	}
	else
	{
		bHasMaterials = false;
	}

	return bHasMaterials;
}
