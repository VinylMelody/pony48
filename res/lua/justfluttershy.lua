--Lua fx file for song "Just... Fluttershy"
--Pony48 source - justfluttershy.lua
--Copyright (c) 2014 Mark Hutcheson

local lasttime
local mid2startcounter
local mid2currow
local mid2rowdir
local mid2humtime
local stuttering

local function jf_init()
	lasttime = -0.001
	mid2humtime = 0.7258
	stuttering = false
	showparticles("bgrdu", true)
	showparticles("bgrdd", true)
	showparticles("bgrdl", true)
	showparticles("bgrdr", true)
end
setglobal("jf_init", jf_init)

local function start(first)
	if first == true then
		showparticles("bgflash", false)
		pinwheelspeed(40)
		fireparticles("bgrdu", false)
		fireparticles("bgrdd", false)
		fireparticles("bgrdl", false)
		fireparticles("bgrdr", false)
	end
end

local function mid1(first)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(120)
	end
end

local function setrowcol(row, dir)
	dir = dir % 4
	row = row % 4
	if dir == 0 then		--from top
		for i = 0, 3 do
			settilecol(i + row*4, 1, 1, 1, 0.5)
			settilebgcol(i + row*4, 0, 0, 0, 0.5)
		end
	elseif dir == 1 then	--from right
		for i = 0,12,4 do
			settilecol(i + (3-row), 1, 1, 1, 0.5)
			settilebgcol(i + (3-row), 0, 0, 0, 0.5)
		end
	elseif dir == 2 then	--from bottom
		for i = 0, 3 do
			settilecol(i + (3-row)*4, 1, 1, 1, 0.5)
			settilebgcol(i + (3-row)*4, 0, 0, 0, 0.5)
		end
	else					--from left
		for i = 0,12,4 do
			settilecol(i + row, 1, 1, 1, 0.5)
			settilebgcol(i + row, 0, 0, 0, 0.5)
		end
	end
end

local function mid2(first, curtime)
	if first == true then
		mid2startcounter = curtime - mid2humtime
		mid2currow = -1
		mid2rowdir = -1
		pinwheelspeed(-120)
	end
	if curtime - mid2startcounter >= mid2humtime then
		mid2startcounter = mid2startcounter + mid2humtime
		mid2currow = mid2currow + 1
		mid2rowdir = mid2rowdir + 1
		
		--Reset color of all tiles
		for i = 0, 15 do
			settilecol(i, 1, 1, 1, 1)
			settilebgcol(i, 0.5, 0.5, 0.5, 0.5)
		end
		setrowcol(mid2currow, math.floor(mid2rowdir/4))
	end
end

local function mid3(first, curtime)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(-120)
		mid2startcounter = curtime - mid2humtime
		mid2currow = -1
		mid2rowdir = 1		--Flip direction in middle of pattern. Originally on accident, but it looks cool
	end
	if curtime - mid2startcounter >= mid2humtime then
		mid2startcounter = mid2startcounter + mid2humtime
		mid2currow = mid2currow + 1
		mid2rowdir = mid2rowdir + 1
		
		--Reset color of all tiles
		for i = 0, 15 do
			settilecol(i, 1, 1, 1, 1)
			settilebgcol(i, 0.5, 0.5, 0.5, 0.5)
		end
		--Change color of one row at a time
		setrowcol(mid2currow, math.floor(mid2rowdir/4))
	end
end

local function drop(first)
	if first == true then
		showparticles("bgponies", false)
		pinwheelspeed(0)
		--Reset color of all tiles
		for i = 0, 15 do
			settilecol(i, 1, 1, 1, 1)
			settilebgcol(i, 0.5, 0.5, 0.5, 0.5)
		end
	end
end

local function main(first)
	if first == true then
		pinwheelspeed(240)
		showparticles("bgflash", true)
		fireparticles("bgflash", true)
		fireparticles("bgrdu", true)
		fireparticles("bgrdd", true)
		fireparticles("bgrdl", true)
		fireparticles("bgrdr", true)
	end
	if stuttering == true then	--Camera can be panned with a secondary gamepad stick, so deal with that possibility
		setcameraxy(0,0)	--Cause this is called before stutter, it all works out
	end
	stuttering = false
end

local function stutter(first, curtime, starttime, endtime)
	if first == true then
		rumblecontroller(0.5, endtime-starttime)
	end
	setcameraxy(math.random()*0.5 - 0.25, math.random()*0.5 - 0.25)	--Random float between -0.25 and +0.25
	stuttering = true
end

local timetab = {
	{func = start, startat = 0, endat = 34.9},
	{func = mid1, startat = 34.9, endat = 46.4},
	{func = mid2, startat = 46.4, endat = 69.2},
	{func = drop, startat = 69.2, endat = 69.848},
	{func = main, startat = 69.848, endat = 116.08},
	{func = start, startat = 116.308, endat = 140.895},
	{func = mid3, startat = 140.895, endat = 163.863},
	{func = drop, startat = 163.863, endat = 164.395},
	{func = main, startat = 164.395, endat = 210.936},
	{func = stutter, startat = 167.306, endat = 168.714},
	{func = stutter, startat = 173.847, endat = 174.565},
	{func = stutter, startat = 178.943, endat = 180.356},
	{func = stutter, startat = 185.485, endat = 186.180},
	{func = start, startat = 210.936, endat = 224},
}

local function jf_update(curtime)
	for key,val in ipairs(timetab) do
		if curtime > val.startat and curtime < val.endat then
			val.func(lasttime < val.startat or lasttime > val.endat, curtime, val.startat, val.endat)
		end
	end
	lasttime = curtime
end
setglobal("jf_update", jf_update)






























