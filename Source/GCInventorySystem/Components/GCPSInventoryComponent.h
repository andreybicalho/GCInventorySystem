// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PlayerStateComponent.h"
#include "Types/InventoryTypes.h"

#include "GCPSInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemGranted, FGameplayTag, itemName, APlayerState*, playerReference);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, FGameplayTag, itemName, APlayerState*, playerReference);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FGameplayTag, itemName, APlayerState*, playerReference);

/**
 *  Inventory component used to manage the inventory of players during the game.
 *  It covers the basic functions for items and requires that the player implements the GCInventoryInterface,
 *  to apply the desired effects of the Items.
 */
UCLASS()
class GCINVENTORYSYSTEM_API UGCPSInventoryComponent : public UPlayerStateComponent
{
	GENERATED_BODY()

public:

	UGCPSInventoryComponent(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Function called to add an item to the inventory with a specific stack
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void AddItemToInventory(FGameplayTag itemTag, float itemStack);

	// Function called to use a specific stack of items from the inventory
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void UseItemFromInventory(FGameplayTag itemTag, float itemStack);

	// Function called to drop a specific stack of items from the inventory
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void DropItemFromInventory(FGameplayTag itemTag, float itemStack);

	// Function called to remove a specific stack of items from the inventory
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void RemoveItemFromInventory(FGameplayTag itemTag, float itemStack);

	// Returns true if the input item is in the inventory
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	bool IsItemInInventory(FGameplayTag itemTag) const;

	// Returns the current stack of the input item from the inventory. 0 if the player doesn't have the item.
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	float GetItemStack(FGameplayTag itemTag) const;

public:

	UPROPERTY(BlueprintCallable)
	FOnItemGranted OnItemGranted;

	UPROPERTY(BlueprintCallable)
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintCallable)
	FOnItemRemoved OnItemRemoved;

protected:

	// Gameplay tags of the items that the player holds
	UPROPERTY(Replicated)
	FGameplayTagStackContainer HeldItemTags;

	// 	UPROPERTY(EditDefaultsOnly, Category = "InventoryComponent|Defaults")
	// 	FGameplayTagStackContainer StartUpItems;
};
