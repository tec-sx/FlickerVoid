// Small text helpers shared by the menus and the HUD.
namespace FVUIText
{
    // One decimal place, e.g. 12.5.
    FString Decimal(float Value)
    {
        int Tenths = Math::RoundToInt(Math::Abs(Value) * 10.0);
        FString Sign = Value < 0.0 && Tenths > 0 ? "-" : "";
        return f"{Sign}{Math::IntegerDivisionTrunc(Tenths, 10)}.{Tenths % 10}";
    }

    FText Weight(float Kilograms)
    {
        return FText::FromString(f"{Decimal(Kilograms)} kg");
    }

    FText Ratio(FString Current, FString Limit)
    {
        return FText::FromString(f"{Current} / {Limit}");
    }

    // Last part of a tag, e.g. "Outfit" for Item.Outfit.
    FText TagLeaf(FGameplayTag Tag)
    {
        TArray<FString> Parts;
        Tag.GetTagName().ToString().ParseIntoArray(Parts, ".");
        return Parts.Num() > 0 ? FText::FromString(Parts.Last()) : FText();
    }

    float Fraction(float Value, float Max)
    {
        return Max > 0.0 ? Math::Clamp(Value / Max, 0.0, 1.0) : 0.0;
    }

    // Where a scale's current value sits in its range from the Facts settings; 0 when the scale has no range.
    float ScaleFraction(UFVScaleDefinition Scale)
    {
        int Min = 0;
        int Max = 0;
        if (!FVScale::GetRange(Scale, Min, Max))
        {
            return 0.0;
        }

        int Value = FVScale::GetValue(Scale);
        return Fraction(float(Value - Min), float(Max - Min));
    }
}
