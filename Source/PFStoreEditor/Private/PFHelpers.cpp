#include "PFHelpers.h"

#include "StoreItemProvider.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PFHelpers
{
	PlayFab::AdminModels::FCatalogItemConsumableInfo
		ToPlayFabConsumableInfo(const FConsumableInfo& In)
	{
		PlayFab::AdminModels::FCatalogItemConsumableInfo Out;

		if (In.UsageCount > 0)
		{
			Out.UsageCount = PlayFab::Boxed<uint32>(static_cast<uint32>(In.UsageCount));
		}

		if (In.UsagePeriod > 0)
		{
			Out.UsagePeriod = PlayFab::Boxed<uint32>(static_cast<uint32>(In.UsagePeriod));
		}

		if (!In.UsagePeriodGroup.IsEmpty())
		{
			Out.UsagePeriodGroup = In.UsagePeriodGroup;
		}

		return Out;
	}

	static FString EscapeCsv(const FString& In)
	{
		FString Out = In.Replace(TEXT("\""), TEXT("\"\""));
		return FString::Printf(TEXT("\"%s\""), *Out);
	}

	static const TCHAR* BoolStr(bool b) { return b ? TEXT("TRUE") : TEXT("FALSE"); }

	bool ExportToCsv(const TArray<TWeakObjectPtr<UObject>>& Items, const FString& FilePath)
	{
		TArray<FString> Lines;

		Lines.Add(TEXT("ItemId,DisplayName,ItemClass,Description,CustomData,Tags,")
			TEXT("IsLimitedEdition,IsTokenForCharacterCreation,IsTradable,IsStackable,")
			TEXT("UsageCount,UsagePeriod,UsagePeriodGroup,")
			TEXT("BundledItems,BundledResultTables,BundledVirtualCurrencies,")
			TEXT("KeyItemId,ItemContents,ResultTableContents,VirtualCurrencyContents"));

		for (const TWeakObjectPtr<UObject>& ItemPtr : Items)
		{
			UObject* Obj = ItemPtr.Get();
			if (!Obj) continue;

			if (!Obj->GetClass()->ImplementsInterface(UStoreItemProvider::StaticClass()))
			{
				continue;
			}

			IStoreItemProvider* Provider = Cast<IStoreItemProvider>(Obj);
			if (!Provider) continue;

			const FString ItemId = EscapeCsv(Provider->GetItemId());
			const FString Name = EscapeCsv(Provider->GetDisplayName());
			const FString ClassStr = EscapeCsv(Provider->GetItemClass());
			const FString Desc = EscapeCsv(Provider->GetDescription());
			const FString Custom = EscapeCsv(Provider->GetCustomData());

			FString Tags;
			{
				const TArray<FString> RawTags = Provider->GetTags();
				Tags = EscapeCsv(FString::Join(RawTags, TEXT(";")));
			}

			const FString IsLimited = BoolStr(Provider->GetIsLimitedEdition());
			const FString IsToken = BoolStr(Provider->GetIsTokenForCharacterCreation());
			const FString IsTradable = BoolStr(Provider->GetIsTradable());
			const FString IsStackable = BoolStr(Provider->GetIsStackable());

			// Consumable
			const FConsumableInfo CI = Provider->GetConsumableInfo();
			const FString UsageCountStr = (CI.UsageCount > 0) ? FString::FromInt(CI.UsageCount) : TEXT("");
			const FString UsagePeriodStr = (CI.UsagePeriod > 0) ? FString::FromInt(CI.UsagePeriod) : TEXT("");
			const FString UsageGroupStr = EscapeCsv(CI.UsagePeriodGroup);

			// Bundle
			FString BundledItemsStr, BundledResultTablesStr, BundledVirtualCurrenciesStr;
			if (Obj->GetClass()->ImplementsInterface(UStoreBundleProvider::StaticClass()))
			{
				IStoreBundleProvider* BundleProvider = Cast<IStoreBundleProvider>(Obj);
				if (BundleProvider)
				{
					const FBundleInfo BI = BundleProvider->GetBundleInfo();

					BundledItemsStr = EscapeCsv(FString::Join(BI.BundledItems, TEXT(";")));
					BundledResultTablesStr = EscapeCsv(FString::Join(BI.BundledResultTables, TEXT(";")));

					if (BI.BundledVirtualCurrencies.Num() > 0)
					{
						TArray<FString> Pairs;
						Pairs.Reserve(BI.BundledVirtualCurrencies.Num());
						for (const auto& KV : BI.BundledVirtualCurrencies)
						{
							Pairs.Add(KV.Key + TEXT(":") + FString::FromInt(KV.Value));
						}
						BundledVirtualCurrenciesStr = EscapeCsv(FString::Join(Pairs, TEXT(";")));
					}
					else
					{
						BundledVirtualCurrenciesStr = TEXT("");
					}
				}
			}
			else
			{
				BundledItemsStr = BundledResultTablesStr = BundledVirtualCurrenciesStr = TEXT("");
			}

			// Container
			FString KeyItemIdStr, ItemContentsStr, ResultTableContentsStr, VirtualCurrencyContentsStr;
			if (Obj->GetClass()->ImplementsInterface(UStoreContainerProvider::StaticClass()))
			{
				IStoreContainerProvider* ContainerProvider = Cast<IStoreContainerProvider>(Obj);
				if (ContainerProvider)
				{
					const FContainerInfo ContainerInfo = ContainerProvider->GetContainerInfo();

					KeyItemIdStr = EscapeCsv(ContainerInfo.KeyItemId);
					ItemContentsStr = EscapeCsv(FString::Join(ContainerInfo.ItemContents, TEXT(";")));
					ResultTableContentsStr = EscapeCsv(FString::Join(ContainerInfo.ResultTableContents, TEXT(";")));

					if (ContainerInfo.VirtualCurrencyContents.Num() > 0)
					{
						TArray<FString> Pairs;
						Pairs.Reserve(ContainerInfo.VirtualCurrencyContents.Num());
						for (const auto& KV : ContainerInfo.VirtualCurrencyContents)
						{
							Pairs.Add(KV.Key + TEXT(":") + FString::FromInt(KV.Value));
						}
						VirtualCurrencyContentsStr = EscapeCsv(FString::Join(Pairs, TEXT(";")));
					}
					else
					{
						VirtualCurrencyContentsStr = TEXT("");
					}
				}
			}
			else
			{
				KeyItemIdStr = ItemContentsStr = ResultTableContentsStr = VirtualCurrencyContentsStr = TEXT("");
			}

			const FString Line = FString::Printf(
				TEXT("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s"),
				*ItemId, *Name, *ClassStr, *Desc, *Custom, *Tags,
				*IsLimited, *IsToken, *IsTradable, *IsStackable,
				*UsageCountStr, *UsagePeriodStr, *UsageGroupStr,
				*BundledItemsStr, *BundledResultTablesStr, *BundledVirtualCurrenciesStr,
				*KeyItemIdStr, *ItemContentsStr, *ResultTableContentsStr, *VirtualCurrencyContentsStr
			);

			Lines.Add(Line);
		}

		const FString CsvContent = FString::Join(Lines, TEXT("\r\n"));
		return FFileHelper::SaveStringToFile(CsvContent, *FilePath);
	}

	void ParseCsvLine(const FString& Line, TArray<FString>& OutFields)
	{
		OutFields.Empty();

		FString Current;
		bool bInQuotes = false;

		const int32 Len = Line.Len();
		for (int32 i = 0; i < Len; ++i)
		{
			const TCHAR C = Line[i];

			if (C == '"')
			{
				if (bInQuotes && i + 1 < Len && Line[i + 1] == '"')
				{
					Current += '"';
					++i;
				}
				else
				{
					bInQuotes = !bInQuotes;
				}
			}
			else if (C == ',' && !bInQuotes)
			{
				FString Field = Current.TrimStartAndEnd();
				if (Field.StartsWith(TEXT("\"")) && Field.EndsWith(TEXT("\"")) && Field.Len() >= 2)
				{
					Field = Field.Mid(1, Field.Len() - 2);
				}

				OutFields.Add(Field);
				Current.Empty();
			}
			else
			{
				Current += C;
			}
		}

		FString Field = Current.TrimStartAndEnd();
		if (Field.StartsWith(TEXT("\"")) && Field.EndsWith(TEXT("\"")) && Field.Len() >= 2)
		{
			Field = Field.Mid(1, Field.Len() - 2);
		}
		OutFields.Add(Field);
	}

	FString Unescape(const FString& In)
	{
		return In;
	}

	bool ImportItemsFromCsv(const FString& FilePath, TArray<PlayFab::AdminModels::FCatalogItem>& OutItems)
	{
		OutItems.Empty();

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load CSV: %s"), *FilePath);
			return false;
		}
		if (Lines.Num() < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("CSV has no data lines."));
			return false;
		}

		for (int32 i = 1; i < Lines.Num(); ++i)
		{
			const FString Line = Lines[i].TrimStartAndEnd();
			if (Line.IsEmpty()) continue;

			TArray<FString> Fields;
			ParseCsvLine(Line, Fields);
			if (Fields.Num() < 20) continue;

			PlayFab::AdminModels::FCatalogItem Item;

			Item.ItemId = Fields[0];
			Item.DisplayName = Fields[1];
			Item.ItemClass = Fields[2];
			Item.Description = Fields[3];
			Item.CustomData = Fields[4];

			// Tags
			{
				TArray<FString> Tags;
				Fields[5].ParseIntoArray(Tags, TEXT(";"), true);
				Item.Tags = Tags;
			}

			Item.IsLimitedEdition = Fields[6].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			Item.CanBecomeCharacter = Fields[7].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			Item.IsTradable = Fields[8].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			Item.IsStackable = Fields[9].Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);

			// Consumable
			{
				auto CI = MakeShared<PlayFab::AdminModels::FCatalogItemConsumableInfo>();
				const FString& SC = Fields[10];
				const FString& SP = Fields[11];

				if (!SC.IsEmpty()) CI->UsageCount = PlayFab::Boxed<uint32>(static_cast<uint32>(FCString::Atoi(*SC)));
				if (!SP.IsEmpty()) CI->UsagePeriod = PlayFab::Boxed<uint32>(static_cast<uint32>(FCString::Atoi(*SP)));

				CI->UsagePeriodGroup = Fields[12];
				Item.Consumable = CI;
			}

			// Bundle
			{
				auto BI = MakeShared<PlayFab::AdminModels::FCatalogItemBundleInfo>();

				TArray<FString> Arr;
				Fields[13].ParseIntoArray(Arr, TEXT(";"), true);
				BI->BundledItems = Arr;

				Arr.Reset();
				Fields[14].ParseIntoArray(Arr, TEXT(";"), true);
				BI->BundledResultTables = Arr;

				if (!Fields[15].IsEmpty())
				{
					TArray<FString> Pairs;
					Fields[15].ParseIntoArray(Pairs, TEXT(";"), true);
					for (const FString& P : Pairs)
					{
						FString Key, Val;
						if (P.Split(TEXT(":"), &Key, &Val))
						{
							BI->BundledVirtualCurrencies.Add(Key, FCString::Atoi(*Val));
						}
					}
				}

				Item.Bundle = BI;
			}

			// Container
			{
				auto CI = MakeShared<PlayFab::AdminModels::FCatalogItemContainerInfo>();
				CI->KeyItemId = Fields[16];

				TArray<FString> Arr;
				Fields[17].ParseIntoArray(Arr, TEXT(";"), true);
				CI->ItemContents = Arr;

				Arr.Reset();
				Fields[18].ParseIntoArray(Arr, TEXT(";"), true);
				CI->ResultTableContents = Arr;

				if (!Fields[19].IsEmpty())
				{
					TArray<FString> Pairs;
					Fields[19].ParseIntoArray(Pairs, TEXT(";"), true);
					for (const FString& P : Pairs)
					{
						FString Key, Val;
						if (P.Split(TEXT(":"), &Key, &Val))
						{
							CI->VirtualCurrencyContents.Add(Key, FCString::Atoi(*Val));
						}
					}
				}

				Item.Container = CI;
			}

			// Defaults
			Item.CatalogVersion = TEXT("Main");
			Item.ItemImageUrl = FString();
			Item.InitialLimitedEditionCount = 0;
			Item.RealCurrencyPrices.Empty();

			OutItems.Add(MoveTemp(Item));
		}

		return true;
	}

	bool ImportDropTablesFromCsv(const FString& FilePath, TArray<PlayFab::AdminModels::FRandomResultTable>& OutItems)
	{
		OutItems.Empty();

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load CSV: %s"), *FilePath);
			return false;
		}
		if (Lines.Num() < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("CSV has no data lines."));
			return false;
		}

		for (int32 i = 1; i < Lines.Num(); ++i)
		{
			const FString Line = Lines[i].TrimStartAndEnd();
			if (Line.IsEmpty()) continue;

			PlayFab::AdminModels::FRandomResultTable Item;
			OutItems.Add(MoveTemp(Item));
		}

		return true;
	}
}