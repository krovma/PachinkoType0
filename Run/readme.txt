Screen Display
    Top:
        Obj: Object counts
        Mass: Mass for next created object.
        Bounce: Bounciness for next created object.
        Friction: Friction for next created object.

    Green Text:
        Information about the nearest object to the cursor
        Mass, Inertia: as named
        Res: bounciness
        v: linear velocity
        a: linear acceleration
        drag: linear drag
        rot: rotation
        aV: angular velocity
        aA: angular acceration
        drag: angular drag
        restriction: object's moving restriction on x,y and rotation

    Bottom:
        Type: Type for next created object.
        Sim: Simulation type for next created object.
        Trigger: Whether next object is a trigger (overlapping events only) or not.
            Currently, capsule trigger is working as dead zone, and box trigger will show a text in the console.
Controls
    [Mouse Left Button Drag]: Create new object, in random radius/width.
    [F3]: Toggle auto mass
    [F1]: Toggle next object's shape (Box and Capsule)
    [F2]: Toggle static object or Dynamic object
    [G]:  Toggle trigger object or solid object
    [Mouse Right Button] and hold: pick an object
        [Right Button Drag] while picking: move the object
        [Delete] while picking: remove picked object
    [N]/[M]: Change default mass or picked object's mass
    [,]/[.]: Change default bounciness or picked object's bounciness
    [;]/[']: Change default friction or picked object's friction
    [-]/[=]: Change default linear drag or picked object's linear drag
    [9]/[0]: Change default angular drag or picked object's angular drag
    [X]/[Y]/[Z]: Change selected object's restriction. X-axis, Y-axis and rotating respectively.
    [W]/[A]/[S]/[D]: Moving camera
    [Wheel Up]/[Wheel Down]: Zoom in/out.

    [/]: Open console
        commands
        save [file=<filename>]
            Default: <filename> = dafaultMap.xml
            Example: save file=Map0.xml
            Example: save
            Save current scene to Run/Data/Map/<filename>
        load [file=<filename>]
            Default: <filename> = defaultMap.xml
            Example: load file=Map0.xml
            Example: load
            Load the scene from Run/Data/Map/<filename>

