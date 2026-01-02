// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OverHeadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/CAttributeSet.h"

void UOverHeadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitysystemComponent)
{
	
	if (AbilitysystemComponent)
	{
		HealthBar->SetAndboundToGameplayAttribute(AbilitysystemComponent, UCAttributeSet::GetHealthAttribute(), UCAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndboundToGameplayAttribute(AbilitysystemComponent,  UCAttributeSet::GetManaAttribute(), UCAttributeSet::GetMaxManaAttribute());
		
	}
}
