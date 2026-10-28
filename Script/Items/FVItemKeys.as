// Keys open locks: an item whose definition Tags contain the lock's Key Tag (or a child of it).
namespace FVItemKeys
{
	// The holder's first inventory stack that fits the lock.
	bool FindKeyFor(AActor Holder, UFVLockComponent Lock, FFVItemStack&out OutKey)
	{
		if (Holder == nullptr || !IsValid(Lock) || !Lock.GetKeyTag().IsValid())
		{
			return false;
		}

		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Holder);

		return IsValid(Inventory) && Inventory.FindItemWithTag(Lock.GetKeyTag(), OutKey);
	}

	bool HasKeyFor(AActor Holder, UFVLockComponent Lock)
	{
		FFVItemStack Key;
		return FindKeyFor(Holder, Lock, Key);
	}

	// Unlocks a locked lock with a key the holder carries; a lock that consumes its key takes one unit of it.
	bool TryUnlockWithKey(AActor Holder, UFVLockComponent Lock)
	{
		FFVItemStack Key;

		if (!IsValid(Lock) || !Lock.CanBeOpenedWithKey() || !FindKeyFor(Holder, Lock, Key))
		{
			return false;
		}

		Lock.Unlock(Holder);

		if (Lock.ConsumesKey())
		{
			UFVInventoryComponent::Get(Holder).RemoveStack(Key.InstanceId, 1);
		}

		return true;
	}
}
