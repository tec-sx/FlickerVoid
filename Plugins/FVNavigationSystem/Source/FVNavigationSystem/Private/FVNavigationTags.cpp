#include "FVNavigationTags.h"

namespace FVNavigationTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Map, "Map", "Ids of map definitions.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Map_World, "Map.World", "The whole game world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Map_Region, "Map.Region", "Districts, towns and areas inside the world map.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Map_Interior, "Map.Interior", "Buildings and other interiors with their own floors.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Marker_Type, "Marker.Type", "Ids of marker definitions.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Marker_Type_Waypoint, "Marker.Type.Waypoint", "The waypoint the player places.");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Type_QuestTarget, "Marker.Type.QuestTarget");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Type_QuestGiver, "Marker.Type.QuestGiver");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Type_PointOfInterest, "Marker.Type.PointOfInterest");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Type_Shop, "Marker.Type.Shop");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Type_FastTravel, "Marker.Type.FastTravel");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Marker_Category, "Marker.Category", "Marker categories for legend filters; put them in a marker definition's Tags.");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Category_Player, "Marker.Category.Player");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Category_Quest, "Marker.Category.Quest");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Category_Exploration, "Marker.Category.Exploration");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Category_Service, "Marker.Category.Service");
	UE_DEFINE_GAMEPLAY_TAG(Marker_Category_Travel, "Marker.Category.Travel");
}
