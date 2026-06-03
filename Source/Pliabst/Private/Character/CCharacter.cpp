// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACCharacter::ACCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>("CAttribute Set");
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());


}
void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
	CAbilitySystemComponent->ApplyInitialEffects();
	CAbilitySystemComponent->GiveInitialAbilitiies();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool ACCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if(NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}


// Called when the game starts or when spawned
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatusWidget();

	BindGASChangeDelegates();
	
}

// Called every frame
void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

void ACCharacter::BindGASChangeDelegates()
{
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &ACCharacter::DeathTagUpdated);
	}
}

//void ACCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
//{
//	if (NewCount != 0)
//	{
//		StartDeathSequence();
//	}
//	else
//	{
//		Respawn();
//	}
//}



void ACCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		if (!bIsDead)
		{
			bIsDead = true;
			StartDeathSequence();
		}
	}
	else
	{
		bIsDead = false;
		Respawn();
	}
}


void ACCharacter::ConfigureOverHeadStatusWidget()
{
	if (!OverHeadWidgetComponent)
	{

		return;
	}

	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	UOverHeadStatsGauge* OverHeadStatusGauge =Cast <UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverHeadStatusGauge)
	{
		OverHeadStatusGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle, this, &ACCharacter::UpdateHeadGaugeVisibility, HeadStatGaugeVisibilityCheckUpdateGap, true);
	}
}

//void ACCharacter::UpdateHeadGaugeVisibility()
//{
//	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
//	if (LocalPlayerPawn)
//	{
//		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
//		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
//	}
//}

void ACCharacter::UpdateHeadGaugeVisibility()
{
	if (!OverHeadWidgetComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverHeadWidgetComponent is null in UpdateHeadGaugeVisibility on %s"), *GetName());
		return;
	}

	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn)
	{
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}


void ACCharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	// If the actor has no valid world (being destroyed / not initialized), bail out
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetStatusGaugeEnabled called on %s with no valid World"), *GetName());
		return;
	}

	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);

	if (bIsEnabled)
	{
		ConfigureOverHeadStatusWidget();
	}
	else
	{
		if (OverHeadWidgetComponent)
		{
			OverHeadWidgetComponent->SetHiddenInGame(true);
		}
		/*else
		{
			UE_LOG(LogTemp, Warning, TEXT("OverHeadWidgetComponent is null in SetStatusGaugeEnabled on %s"),
				*GetName());
		}*/
	}
}


void ACCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	
}

//void ACCharacter::StartDeathSequence()
//{
//	UE_LOG(LogTemp, Warning, TEXT("Dead"));
//	OnDead();
//	/*if (CAbilitySystemComponent)
//	{
//		CAbilitySystemComponent->CancelAllAbilities();
//	}*/
//
//	PlayDeathAnimation();
//	SetStatusGaugeEnabled(false);
//
//	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
//	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//}

void ACCharacter::StartDeathSequence()
{
	UE_LOG(LogTemp, Warning, TEXT("Dead"));

	OnDead(); // if this is a delegate, make sure it’s only bound to valid objects

	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(EMovementMode::MOVE_None);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterMovement is null in StartDeathSequence on %s"), *GetName());
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CapsuleComponent is null in StartDeathSequence on %s"), *GetName());
	}
}


//void ACCharacter::Respawn()
//{
//	UE_LOG(LogTemp, Warning, TEXT("Respawn"));
//	OnRespawn();
//	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
//	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
//	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
//	SetStatusGaugeEnabled(true); //265
//}

void ACCharacter::Respawn()
{
	UE_LOG(LogTemp, Warning, TEXT("Respawn"));

	OnRespawn();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->SetMovementMode(EMovementMode::MOVE_Walking);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
			Anim->StopAllMontages(0.f);
	}

	// Only enable the gauge if the widget exists AND the world exists
	if (OverHeadWidgetComponent && GetWorld())
	{
		SetStatusGaugeEnabled(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping SetStatusGaugeEnabled during Respawn — component or world not ready"));
	}

	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatEffect();
	}
}


void ACCharacter::OnDead()
{
	
}

void ACCharacter::OnRespawn()
{
	
}

