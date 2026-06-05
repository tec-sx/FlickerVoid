# Animation Instance Proxy Patterns

## The Initialization Order Problem

**CRITICAL UNDERSTANDING:** AnimInstanceProxy initialization happens **BEFORE** AnimInstance initialization!

### Initialization Sequence

```cpp
1. UAnimInstance Constructor
   ↓
2. FAnimInstanceProxy::Initialize(UAnimInstance*)  ← TOO EARLY FOR CUSTOM DATA!
   ↓
3. UAnimInstance::NativeInitializeAnimation()      ← Custom setup happens here
   ↓
4. FAnimInstanceProxy::PreUpdate()                 ← SAFE to access custom data
   ↓
5. FAnimInstanceProxy::Update() (Worker Thread)
```

### Why This Matters

```cpp
// ❌ WRONG - Will crash or get null pointers!
void MyProxy::Initialize(UAnimInstance* InAnimInstance)
{
    auto MyAnim = Cast<UMyAnim>(InAnimInstance);
    Character = MyAnim->CachedCharacter;  // NULL! Not set yet!
    Config = MyAnim->Config;              // NULL! Not set yet!
}

// ✅ CORRECT - Lazy initialization in PreUpdate
void MyProxy::Initialize(UAnimInstance* InAnimInstance)
{
    FAnimInstanceProxy::Initialize(InAnimInstance);
    // Don't access custom AnimInstance data here!
}

void MyProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
    FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
    
    // Lazy init - only first frame
    if (!Character.IsValid())
    {
        auto MyAnim = Cast<UMyAnim>(InAnimInstance);
        Character = MyAnim->CachedCharacter;  // ✅ SAFE NOW!
        Config = MyAnim->Config;              // ✅ SAFE NOW!
    }
    
    // Normal per-frame work...
}
```

## Epic's Recommended Patterns

### Pattern 1: Lazy Initialization (Recommended)

Used by Epic in most engine animation systems.

```cpp
// PROXY
void FMyAnimProxy::Initialize(UAnimInstance* InAnimInstance)
{
    FAnimInstanceProxy::Initialize(InAnimInstance);
    // Don't cache anything custom here
}

void FMyAnimProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
    FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
    
    // First-time setup (runs once)
    if (!bIsInitialized)
    {
        UMyAnimInstance* MyAnim = Cast<UMyAnimInstance>(InAnimInstance);
        if (MyAnim && MyAnim->CachedCharacter)
        {
            CharacterPtr = MyAnim->CachedCharacter;
            ConfigPtr = MyAnim->Config;
            bIsInitialized = true;
            
            UE_LOG(LogAnimation, Log, TEXT("Proxy initialized"));
        }
    }
    
    // Per-frame work
    if (bIsInitialized)
    {
        // Safe to use CharacterPtr, ConfigPtr here
    }
}

// HEADER
bool bIsInitialized = false;
```

**Pros:**
- ✅ Safe and reliable
- ✅ Minimal overhead (one bool check per frame)
- ✅ Epic's standard pattern

**Cons:**
- First frame might skip work (acceptable)

### Pattern 2: Every-Frame Cache Refresh (Alternative)

Don't cache at all - get fresh pointers every PreUpdate.

```cpp
void FMyAnimProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
    FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
    
    // Get fresh pointers every frame (no caching)
    UMyAnimInstance* MyAnim = Cast<UMyAnimInstance>(InAnimInstance);
    if (!MyAnim || !MyAnim->CachedCharacter)
    {
        return;
    }
    
    // Use directly - no member variables
    FFVCharacterAnimationData Data = MyAnim->CachedCharacter->GetAnimationData();
    // ... work with data
}
```

**Pros:**
- ✅ Always up-to-date (no stale pointers)
- ✅ Simpler (no init flag)

**Cons:**
- ❌ Repeated Cast every frame (minor overhead)
- ❌ Can't access cached data in Update() safely

### Pattern 3: Separated Initialization (Complex Systems)

For complex proxies with lot of setup.

```cpp
// PROXY HEADER
class FMyAnimProxy : public FAnimInstanceProxy
{
    bool bCharacterDataValid = false;
    
    void InitializeCharacterData(UMyAnimInstance* InAnimInstance);
    void UpdateCharacterData(UMyAnimInstance* InAnimInstance);
};

// PROXY CPP
void FMyAnimProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
    FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
    
    UMyAnimInstance* MyAnim = Cast<UMyAnimInstance>(InAnimInstance);
    if (!MyAnim)
    {
        return;
    }
    
    // One-time initialization
    if (!bCharacterDataValid)
    {
        InitializeCharacterData(MyAnim);
    }
    
    // Per-frame update
    if (bCharacterDataValid)
    {
        UpdateCharacterData(MyAnim);
    }
}

void FMyAnimProxy::InitializeCharacterData(UMyAnimInstance* AnimInstance)
{
    if (!AnimInstance->CachedCharacter)
    {
        return;
    }
    
    CharacterPtr = AnimInstance->CachedCharacter;
    ConfigPtr = AnimInstance->Config;
    
    // Complex setup...
    
    bCharacterDataValid = true;
    UE_LOG(LogAnimation, Log, TEXT("Character data initialized"));
}

void FMyAnimProxy::UpdateCharacterData(UMyAnimInstance* AnimInstance)
{
    // Per-frame character data update
    CharacterData = CharacterPtr->GetAnimationData();
    // ... more work
}
```

**Pros:**
- ✅ Clean separation of concerns
- ✅ Good for complex initialization

**Cons:**
- More boilerplate

## Thread Safety Rules

### Game Thread Context (Safe)
```cpp
✅ PreUpdate()        - Can access Character directly
✅ NativeUpdate()     - Can access Character directly
✅ AnimInstance       - Always game thread
```

### Worker Thread Context (UNSAFE!)
```cpp
❌ Update()           - CANNOT access Character/Components
✅ Update()           - CAN use cached data from PreUpdate
```

### Safe Pattern
```cpp
void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
    // GAME THREAD - Safe to access character
    CharacterData = Character->GetAnimationData();  // ✅ SAFE
}

void Update(float DeltaSeconds)
{
    // WORKER THREAD - Use cached data only
    float Speed = CharacterData.GroundSpeed;        // ✅ SAFE
    
    // Character->GetVelocity()                     // ❌ CRASH!
}
```

## Our Implementation (FVCharacterAnimInstance)

We use **Pattern 1: Lazy Initialization** for optimal performance and safety.

### Key Points

1. **Initialize()** - Only calls parent, no custom setup
2. **PreUpdate()** - Lazy init on first frame, then copy character data
3. **Update()** - Uses cached data (thread-safe)
4. **NativeUpdateAnimation()** - Detects tag changes, calls Blueprint events

### Code Flow

```cpp
// Frame 1
Initialize()              - No custom work
NativeInitializeAnimation() - Sets CachedCharacter, AnimationConfig
PreUpdate()              - Detects nulls, caches character/config
Update()                 - Uses cached data

// Frame 2+
PreUpdate()              - Skips init check (already valid), copies data
Update()                 - Uses cached data
```

## Common Mistakes

### ❌ Mistake 1: Accessing AnimInstance Custom Data in Initialize()
```cpp
void MyProxy::Initialize(UAnimInstance* InAnimInstance)
{
    auto MyAnim = Cast<UMyAnim>(InAnimInstance);
    Character = MyAnim->CachedCharacter;  // NULL! Too early!
}
```

### ❌ Mistake 2: Accessing Character in Update()
```cpp
void MyProxy::Update(float DeltaSeconds)
{
    // WORKER THREAD - Character access is UNSAFE!
    float Speed = Character->GetVelocity().Size();  // CRASH!
}
```

### ❌ Mistake 3: Not Checking Validity
```cpp
void MyProxy::PreUpdate(...)
{
    CharacterData = CharacterPtr->GetAnimationData();  // CRASH if null!
}
```

### ✅ Correct Versions

```cpp
// ✅ Correct 1: Lazy init in PreUpdate
void MyProxy::PreUpdate(...)
{
    if (!CharacterPtr.IsValid())
    {
        // Initialize from AnimInstance
    }
}

// ✅ Correct 2: Use cached data in Update
void MyProxy::Update(float DeltaSeconds)
{
    float Speed = CachedSpeed;  // From PreUpdate
}

// ✅ Correct 3: Always check validity
void MyProxy::PreUpdate(...)
{
    if (CharacterPtr.IsValid())
    {
        CharacterData = CharacterPtr->GetAnimationData();
    }
}
```

## Performance Considerations

### Lazy Init Cost
```cpp
// Per frame check (trivial)
if (!bIsInitialized)  // ~1 CPU cycle
{
    // Init code (runs once)
}
```

### Cache vs Fresh
```cpp
// Cached (faster)
CharacterPtr  // Set once, used many times

// Fresh (slower but always valid)
Cast<UMyAnim>(InAnimInstance)->CachedCharacter  // Cast + dereference every frame
```

### Recommendation
- Use **lazy init with cached pointers** (our implementation)
- Check is trivial (~1 cycle/frame)
- Cached access is fastest

## References

- Epic's FAnimNode_Base implementations
- UE5 Source: Runtime/AnimGraphRuntime/Private/*
- Control Rig: CustomizableObjectSystem animation proxies

## Summary

**DO:**
- ✅ Lazy initialize in PreUpdate
- ✅ Copy character data in PreUpdate
- ✅ Use cached data in Update
- ✅ Check pointer validity
- ✅ Keep Update thread-safe

**DON'T:**
- ❌ Access custom AnimInstance data in Initialize
- ❌ Access character/components in Update
- ❌ Assume pointers are valid
- ❌ Do heavy work on game thread

**Our pattern is Epic's recommended approach and matches engine animation systems!**

