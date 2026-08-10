#include "FactDB/Debugger/FVFactDebuggerStyle.h"

#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FFVFactDebuggerStyle> FFVFactDebuggerStyle::StyleInstance;

void FFVFactDebuggerStyle::Register()
{
	check(StyleInstance.IsValid() == false);
	StyleInstance = MakeShared<FFVFactDebuggerStyle>();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FFVFactDebuggerStyle::Unregister()
{
	check(StyleInstance.IsValid());
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	StyleInstance.Reset();
}

FFVFactDebuggerStyle& FFVFactDebuggerStyle::Get()
{
	check(StyleInstance.IsValid());
	return *StyleInstance;
}

FName FFVFactDebuggerStyle::GetStyleSetName()
{
	static FName FactDebuggerStyleName(TEXT("FactDebuggerStyle"));
	return FactDebuggerStyleName;
}

FFVFactDebuggerStyle::FFVFactDebuggerStyle() : FSlateStyleSet(GetStyleSetName())
{
	FSlateStyleSet::SetContentRoot(
		FPaths::Combine(FPaths::GameSourceDir(), TEXT("FlickerVoidCoreEditor")) / TEXT("Resources"));

	Set("ClassIcon.FactPreset", new IMAGE_BRUSH_SVG(TEXT("Icons/FactPreset"), CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.FactPreset", new IMAGE_BRUSH_SVG(TEXT("Icons/FactPreset"), CoreStyleConstants::Icon64x64));
	Set("Icons.LeafFacts", new IMAGE_BRUSH_SVG("Icons/LeafFacts", CoreStyleConstants::Icon16x16));
	Set("Icons.DefinedFacts", new IMAGE_BRUSH_SVG("Icons/DefinedFacts", CoreStyleConstants::Icon16x16));
	Set("Icons.Star.Outline", new IMAGE_BRUSH_SVG("Icons/StarOutline", CoreStyleConstants::Icon16x16));
	Set("Icons.Star.OutlineFilled", new IMAGE_BRUSH_SVG("Icons/StarOutlineFilled", CoreStyleConstants::Icon16x16));
	Set("Icons.Reset", new IMAGE_BRUSH_SVG("Icons/Reset", CoreStyleConstants::Icon16x16));

	Set("RichText.StarOutline", FInlineTextImageStyle()
	                            .SetImage(IMAGE_BRUSH_SVG("Icons/StarOutline", CoreStyleConstants::Icon16x16))
	                            .SetBaseline(0));

	Set("Colors.FactChanged", FLinearColor{0.1f, 0.5f, 0.1f, 0.2f});
	Set("Colors.FactPreset", FColor(243, 113, 42));

	Set("NameFont", DEFAULT_FONT("Regular", 9));
	Set("PathFont", DEFAULT_FONT("Light", 10));
}
