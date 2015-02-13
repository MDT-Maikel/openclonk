/**
	Constructor
	Library for objects which are able to construct structures. This library implements
	opening of the construction menu when the player uses the object (e.g. hammer).
	
	@author Maikel
*/


public func IsConstructor() { return true; }

public func ControlUseStart(object clonk, int x, int y)
{
	// Is the clonk able to construct?
	if (clonk->GetProcedure() != "WALK")
	{
		clonk->CancelUse();
		return true;
	}
	clonk->CreateConstructionMenu(this, true);
	clonk->CancelUse();
	return true;
}

public func HoldingEnabled() { return true; }

public func ControlUseHolding(object clonk, int x, int y)
{
	clonk->CancelUse();
	return true;
}


/*-- Construction Preview --*/

public func ShowConstructionPreview(object clonk, id structure_id)
{
	AddEffect("ControlConstructionPreview", clonk, 1, 0, this, nil, structure_id, clonk);
	SetPlayerControlEnabled(clonk->GetOwner(), CON_Aim, true);
	return true;
}

// Construction preview effect: initialize the properties of the shown structure here.
protected func FxControlConstructionPreviewStart(object clonk, proplist effect, int temp, id structure_id, object clonk)
{
	if (temp)
		return FX_OK;

	effect.structure = structure_id;
	effect.flipable = !structure_id->~NoConstructionFlip();
	effect.preview = structure_id->~CreateConstructionPreview(clonk);
	if (!effect.preview) 
		effect.preview = CreateObjectAbove(ConstructionPreviewer, AbsX(clonk->GetX()), AbsY(clonk->GetY()), clonk->GetOwner());
	effect.preview->Set(structure_id, clonk);
	return FX_OK;
}

// Construction preview effect: player controls are forwarded through this effect.
protected func FxControlConstructionPreviewControl(object clonk, proplist effect, int ctrl, int x, int y, int strength, bool repeat, bool release)
{
	// The aiming control does nothing but reposition.
	if (ctrl == CON_Aim)
	{
		effect.preview->Reposition(x, y);
		return true;
	}	
		
	// CON_Use is placement of the structure at the current position of the previewer.
	if (ctrl == CON_Use)
		CreateConstructionSite(clonk, effect.structure, AbsX(effect.preview->GetX()), AbsY(effect.preview->GetY() + effect.preview.dimension_y/2), effect.preview.blocked, effect.preview.direction, effect.preview.stick_to);
	// Movement of the clonk is allowed.
	else if (IsMovementControl(ctrl))
		return false;
	// Flipping: this is actually realized twice. Once as an Extra-Interaction in the clonk, and here. 
	// We don't want the Clonk to get any non-movement controls though, so we handle it here too.
	// This means that actionbar-hotkeys won't work for it. However, clicking the button will.
	else if (IsInteractionControl(ctrl))
	{
		if (release)
			effect.preview->Flip();
		return true;
	}

	// All the other controls close the preview and don't place the site.
	RemoveEffect("ControlConstructionPreview", clonk, effect);
	return true;
}

// Construction preview effect: remove previewer when the effect is removed.
protected func FxControlConstructionPreviewStop(object clonk, proplist effect, int reason, bool temp)
{
	if (temp) 
		return FX_OK;

	effect.preview->RemoveObject();
	SetPlayerControlEnabled(clonk->GetOwner(), CON_Aim, false);
	return FX_OK;
}


/*-- Construction Site --*/

private func CreateConstructionSite(object clonk, id structure_id, int x, int y, bool blocked, int dir, object stick_to)
{
	// Only when the clonk is standing and outdoors.
	if (clonk->GetAction() != "Walk" || clonk->Contained())
		return false;
	// Check if the building can be build here.
	if (structure_id->~RejectConstruction(x, y, clonk)) 
		return false;
	if (blocked)
	{
		CustomMessage("$TxtNoSiteHere$", this, clonk->GetOwner(), nil, nil, RGB(255, 0, 0)); 
		return false;
	}
	// Check if the site is blocked by any of the other construction sites.
	for (var other_site in FindObjects(Find_ID(ConstructionSite)))
	{
		if (!(other_site->GetLeftEdge()  > GetX()+x+structure_id->GetDefWidth()/2  ||
		     other_site->GetRightEdge()  < GetX()+x-structure_id->GetDefWidth()/2  ||
		     other_site->GetTopEdge()    > GetY()+y+structure_id->GetDefHeight()/2 ||
		     other_site->GetBottomEdge() < GetY()+y-structure_id->GetDefHeight()/2 ))
		{
			CustomMessage(Format("$TxtBlocked$",other_site->GetName()), this, clonk->GetOwner(), nil,nil, RGB(255, 0, 0));
			return false;
		}
	}
	
	// Set owner for the constructor 
	SetOwner(clonk->GetOwner());
	// Create construction site
	var site;
	site = CreateObjectAbove(ConstructionSite, x, y, Contained()->GetOwner());
	/* note: this is necessary to have the site at the exact position x,y. Otherwise, for reasons I don't know, the
	   ConstructionSite seems to move 2 pixels downwards (on ConstructionSite::Construction() it is still the
	   original position) which leads to that the CheckConstructionSite function gets different parameters later
	   when the real construction should be created which of course could mean that it returns something else. (#1012)
	   - Newton
	*/
	site->SetPosition(GetX()+x,GetY()+y);
	// Randomize sign rotation.
	site->SetProperty("MeshTransformation", Trans_Mul(Trans_Rotate(RandomX(-30, 30), 0, 1, 0), Trans_Rotate(RandomX(-10, 10), 1, 0, 0)));
	site->PlayAnimation("LeftToRight", 1, Anim_Const(RandomX(0, GetAnimationLength("LeftToRight"))), Anim_Const(500));
	site->Set(structure_id, dir, stick_to);
	
	// Check for material near the construction site or in the constructing clonk.
	var comp, index = 0;
	var w = structure_id->GetDefWidth() + 10;
	var h = structure_id->GetDefHeight() + 10;
	while (comp = GetComponent(nil, index, nil, structure_id))
	{
		// Find needed components and their count.
		var count_needed = GetComponent(comp, nil, nil, structure_id);
		index++;
		
		// First look for stuff in the clonk.
		var materials = FindObjects(Find_ID(comp), Find_Container(clonk));
		// Second look for stuff lying around.
		materials = Concatenate(materials, clonk->FindObjects(Find_ID(comp), Find_NoContainer(), Find_InRect(-w / 2, -h / 2, w, h)));
		// Third look for stuff in nearby lorries and other containers.
		for (var cont in clonk->FindObjects(Find_Or(Find_Func("IsLorry"), Find_Func("IsContainer")), Find_InRect(-w / 2, -h / 2, w, h)))
			materials = Concatenate(materials, FindObjects(Find_ID(comp), Find_Container(cont)));
		// Move materials into the site as needed.
		for (var mat in materials)
		{
			if (count_needed <= 0)
				break;
			mat->Exit();
			mat->Enter(site);
			count_needed--;
		}
	}
	
	// Display a message indicating succesful construction.
	clonk->Message("$TxtConstructions$", structure_id->GetName());
	return true;
}
