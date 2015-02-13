/*-- WoodenBridge --*/

#include Library_Structure


local connectedTo = [];

local connectedRight = false;
local connectedLeft = false;
local connectedHatch = false;

public func IsBridge()
{
	return true;
}

protected func Construction()
{
	 SetCategory(C4D_StaticBack);
	return _inherited(...);
}


func ConnectWithHatch(object hatch)
{
	return !connectedHatch;
}

// Called when the elevator construction site is created
func CombineWith(object other)
{
	if(other->~IsBridge())
	{
		if(other->GetX()<GetX())
		{
			other.connectedRight = true;
			this.connectedLeft = true;
		}
		if(other->GetX()>GetX())
		{
			other.connectedLeft = true;
			this.connectedRight = true;
		}
	}
}

//If this bridge dies, inform connected bridges they are no longer connected to it.
func Destruction()
{
	for(var i; i<GetLength(this.connectedTo); i++)
	{
		if(this.connectedTo[i]->GetX()<GetX()) this.connectedTo[i].connectedRight = false;
		if(this.connectedTo[i]->GetX()>GetX()) this.connectedTo[i].connectedLeft = false;
		
		RemoveArrayValue(this.connectedTo[i].connectedTo, this, true);
	}
}


public func NoConstructionFlip() { return true; }


func CheckBridgeSite(int x, int y, object stick_to)
{
	var anchorPoints = GetAnchorPoints();
	var canConstruct = false;
	
	if(stick_to) canConstruct = true;
	
	for(var i; i<GetLength(anchorPoints); i+=2)
	{
		if(GBackSolid(x+anchorPoints[i], y+anchorPoints[i+1]))
			canConstruct = true;
		if(canConstruct) break;
	}
	
	//if there's too much material at the constructionsite: canConstruct = false.
	if(AreaSolidCount(x-36, y-6, 72, 12) >= 36*3*3) canConstruct = false;
	
	//if it overlaps too much with another bridge: canConstruct = false.
	//I think they should be allowed to overlap a little, because often you only need like half a bridge to close a gap.
	if(FindObject(Find_AtRect(x-2, y-3, 4, 3), Find_Func("IsBridge"))) canConstruct = false;
	
	return canConstruct;
}

private func AreaSolidCount(int x, int y, int wdt, int hgt)
{
        var cx,cy,ascnt=0;
        for (cy=y; cy<y+hgt; cy++)
                for (cx=x; cx<x+wdt; cx++)
                        if (GBackSolid(cx,cy))
                                ascnt++;
        return ascnt;
}

//Returns relative coordinates of the anchor points. (x1, y1, x2, y2, etc...)
func GetAnchorPoints()
{
	return([-30,-6, 30, -6]);
}

func Damage()
{
	//Damaged bridges appear darker. To have at least *some* kind of indicator... :S
	SetClrModulation(RGB(255-GetDamage(),255-GetDamage(), 255-GetDamage()));
	
	//If damage is too high, it breaks! :)
	if(GetDamage()>=100)
	{
		CastObjects(Wood, 1, 30, 0,0, 0, 180);
		CastObjects(WoodenBridgeRuin, 1, 30, 0,0, 0, 180);
		
		//Create global particles, so they don't get removed when the bridge is removed.
		Global->CreateParticle("WoodChip", GetX(), GetY()+5, PV_Random(-15, 15), PV_Random(-13, -6), PV_Random(36 * 3, 36 * 10), Particles_WoodChip(), 20);
		RemoveObject();
		return;
	}
}

func GetStrength() {return 100;}

public func HasSpecialConstructionSiteArea()
{
	var w = GetDefWidth();
	var h = GetDefHeight();
	return Rectangle(-w / 2 - 10, -h * 3, w + 20, h * 3);
}

public func GetSiteDimensions()
{
	return { x = GetDefWidth() + 20, y = GetDefHeight() + 10};
}

public func HasNoConstructionSiteChecks() { return true; }

public func ConstructionCombineWith() { return "IsConnectingBridge"; }

// Called to determine if sticking is possible
public func IsConnectingBridge(object previewer)
{
	if (previewer->GetX() > GetX() && connectedRight) 
		return false;
	if (previewer->GetX() < GetX() && connectedLeft) 
		return false;
	return true;
}

public func ConstructionCombineDirection() { return CONSTRUCTION_STICK_Left | CONSTRUCTION_STICK_Right; }

public func HasSpecialPreviewMovement() { return true; }

public func HasGoodPlacementSpot(int x, int y, object stick_to)
{
	var anchor_points = GetAnchorPoints();
	var can_construct = false;
	if (stick_to) 
		can_construct = true;
	
	for (var i; i < GetLength(anchor_points); i += 2)
	{
		if (GBackSolid(x+anchor_points[i], y+anchor_points[i+1]))
			can_construct = true;
		if(can_construct) break;
	}
	
	//if there's too much material at the constructionsite: canConstruct = false.
	if (AreaSolidCount(x-36, y-6, 72, 12) >= 36*3*3) can_construct = false;
	
	//if it overlaps too much with another bridge: canConstruct = false.
	//I think they should be allowed to overlap a little, because often you only need like half a bridge to close a gap.
	if(FindObject(Find_AtRect(x-2, y-3, 4, 3), Find_Func("IsBridge"))) can_construct = false;
	
	return can_construct;
}


/*-- Properties --*/

local Name = "$Name$";
local Description = "$Description$";
local BlastIncinerate = 2;
local ContactIncinerate = 3;
