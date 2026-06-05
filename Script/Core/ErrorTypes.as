// Error handling and validation types for the FlickerVoid project

// General result pattern for operations that can fail
struct FFVResult
{
    UPROPERTY()
    bool bSuccess = true;

    UPROPERTY()
    FString ErrorMessage;
}

// Result creation helpers
namespace FVResultHelpers
{
    FFVResult Success()
    {
        FFVResult Result;
        Result.bSuccess = true;
        return Result;
    }

    FFVResult Failure(FString Message)
    {
        FFVResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = Message;
        return Result;
    }
}

// Validation helpers
namespace FVValidation
{
    // Component validation
    FFVResult ValidateComponent(UActorComponent Component, FString ComponentName)
    {
        if (Component == nullptr)
        {
            return FVResultHelpers::Failure(ComponentName + " component is null");
        }
        return FVResultHelpers::Success();
    }

    // Object validation
    FFVResult ValidateObject(UObject Object, FString ObjectName)
    {
        if (Object == nullptr)
        {
            return FVResultHelpers::Failure(ObjectName + " is null");
        }
        return FVResultHelpers::Success();
    }

    // Array validation
    FFVResult ValidateArrayNotEmpty(int ArraySize, FString ArrayName)
    {
        if (ArraySize == 0)
        {
            return FVResultHelpers::Failure(ArrayName + " is empty");
        }
        return FVResultHelpers::Success();
    }

    // Numeric range validation
    FFVResult ValidateRange(float Value, float Min, float Max, FString ValueName)
    {
        if (Value < Min || Value > Max)
        {
            return FVResultHelpers::Failure(ValueName + " is out of range [" + Min + ", " + Max + "]: " + Value);
        }
        return FVResultHelpers::Success();
    }
}
