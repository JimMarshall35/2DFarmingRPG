# UI

The game has a declarative UI system where UI can be defined in xml and its behavior scripted with lua. UI programming attempts to follow an MVVM pattern and is loosely inspired by WPF. Layers of UI have viewmodel lua table objects. These contain properties which the UI widgets (the "view") bind to either one way or two way bindings. The overall goal is to never write any "viewmodel" type code in C, which the language really isn't suited to.

## UI Implementation

The game engine defines the XMLUIGameLayer game framework layer type. To initialize one of these you pass it an xml file, below is a minimal examle 

```xml
<UIroot>
	<atlas binary="./WfAssets/ui_atlas.atlas">
	</atlas>
    <screen viewmodelFile="./WfAssets/settings.lua" viewmodelFunction="GetSettingsViewModel">
        
    </screen>
</UIroot>
```

The two important top level elements are ```atlas``` and ```screen```. Atlas can point to a pre-compiled atlas or it can describe multiple image files and the atlas will be compiled at runtime (not recommended):

```xml
<UIroot>
    <atlas>
        <sprite source="./WfAssets/Image/kenney_ui-pack/PNG/Grey/Default/slide_horizontal_color.png" top="0" left="0" width="96" height="16" name="defaultRailHorizontal"/>
        <sprite source="./WfAssets/Image/kenney_ui-pack/PNG/Grey/Default/slide_horizontal_color_section.png" top="0" left="0" width="16" height="16" name="defaultSliderHorizontal"/>
        <font source="./WfAssets/Starzy_Darzy_lowercase_letters.ttf" name="default" options="normal">
            <size type="pts" val="16"/>
            <size type="pts" val="32"/>
        </font>
    </atlas>
    <screen viewmodelFile="./WfAssets/settings.lua" viewmodelFunction="GetSettingsViewModel">
        
    </screen>
</UIroot>
```

The screen element must point to a lua script file and a function that returns the viewmodel. A minimal lua file for the above would look like this:

```lua
-- file "./WfAssets/settings.lua
function GetSettingsViewModel()
    return {
        _
        -- the view model implementation:
        -- - fields
        -- - binding functions
        -- - event handlers
            --- - widget mouse events
            --- - OnInit
            --- - button presses
            --- - GameFrameworkEvent handlers 
    }
end
```

Add widgets as children of the screen element:
- The screen should behave the same same as a canvas type element, with a fixed size that is:
    - immediate children can set a ```dockPoint``` attribute to be positioned at certain points on the screen, valid values are:
        - ```centre```
        - ```middleLeft```
        - ```bottomLeft```
        - ```bottomMiddle```
        - ```bottomRight```
        - ```middleRight```
        - ```topRight```
        - ```topMiddle```
        - ```topLeft``` 
    - they're translated from their position by an offset ```x``` and ```y``` attributes, (specified in pixels)
    - they're surrounded by a given amount of padding (specified in pixels), set with the following attributes (specified in pixels):
        - ```paddingTop```
        - ```paddingBottom```
        - ```paddingLeft```
        - ```paddingRight```

- canvas element can have its size set in various ways,
    - fixed size ie ```height="128px"``` - same as the screen element
    - stretch ie ```height="*"``` - fill all available space in parent
    - stretch fractoin ie ```height="2*"``` - all the parents children have this type of dimension, and each one is a fraction of the total "*" - so if there were two children A and B with width(A) = 1* and  width(B) = 2*, B would have 2/3rds the with of the parent and A would have 1/3rd
    - there is also "auto", where the width and height take up the minimum size that contains their children, but I'm pretty sure this doesn't work properly

- The screen and canvas type widget mentioned above are widgets that arrange their children, in addition to this there is the stack panel widget:
    - Lays its children out next to one another in either a horizontal or vertical stack
        - if the children are of different sizes, the smaller ones will be aligned according to the widgets ```horizontalAlignment``` and ```verticalAlignment``` attributes.
        - ```horizontalAlignment``` can have the values:
            - ```left```
            - ```middle```
            - ```right```
        - ```verticalAlignment``` can have the values:
            - ```top```
            - ```middle```
            - ```bottom```
    - padding is applied
    - width and height of the element is the width and height of the laid out children ie ```auto```

In terms of widgets that actually implement an UI element there are (the names below are the exact names the xml nodes must have):
- ```backgroundbox```
    - draws a background box around its child, scales with 9 panel scaling to have bordered windows
- ```radioGroup``` / ```radioButton```
    - use these two widgets to implement a radio button group
- ```slider```
    - slide it with the mouse
- ```static``` 
    - an image
    - (name from MFC class CStatic)
- ```textButton```
    - a clickable button with text in it
- ```textInput```
    - a text entry field
- ```text```
    - some text

For an exhaustive list of attributes of each widget type, read the source code.

Widgets are composable, that means widgets data objects and functions can be reused to implement more complex widgets. For example the Text Button widget reuses code and structs from the background box and Text widgets.

## Viewmodel Bindings
        
You can create a binding to an attribute in the xml (and hence a property on the widget) using the following syntax:

```xml
<slider paddingBottom="20" val="{ZoomVal}" minVal="1.0" maxVal="2.0"/>
```
In the above example the ```val``` property is bound to the ZoomVal viewmodel property.

The viewmodel must have at the minimum a property called ```ZoomVal_Get``` and can also have one called ```ZoomVal_Set``` to create a two way binding:

```lua
function GetSettingsViewModel()
    return {
        _zoomVal = 0,

        -- two way binding
		Get_ZoomVal = function(self)
			return self._zoomVal
		end,
		Set_ZoomVal = function(self, val)
			self._zoomVal = val
            SetGameLayerZoom(self._zoomVal, self._pGamelayer)
            px, py = WfGetPlayerLocation(self._pGamelayer)
            CenterCameraAt(px, py, self._pGamelayer)
            WfSavePreferences(self._pGamelayer) -- save to the persistant game data object 
		end
    }
end
```

You can also bind to the content of a node like so, this works the same name but the binding gets or sets the text content of the node, used for the text and textInput widgets:

```xml
<textInput font="default" 
                    colour="0,0,0,255" 
                    paddingLeft="20" 
                    width="200px" 
                    fontSize="32pts" 
                    paddingBottom="20" 
                    maxStringLength="128" 
                    onEnter="onTextLineEnter" 
                    horizontalAlignment="left">
    {bindingProp}
</textInput>
```

The stackpanel widget can bind its children to a viewmodel property:

```xml
<stackpanel childrenBinding="InventoryChildren"/>
```


This is really a different kind of binding to the ones above hence it doesn't use the ```{}``` syntax. The viewmodel can dynamically set the children of a widget through this binding. This doesn't exactly adhere to the MVVM pattern, but whatever.

the lua side looks like this:

```lua
    -- in viewmodel table...

    InventoryChildren = function(self)
        self.widgetChildren = {}
        for index, item in pairs(self._items) do
            local spriteName = "no-item"
            if item.item >= 0 then
                spriteName = WfGetItemSpriteName(item.item)
            end
            local backgroundBoxSpriteName = "fantasy_9Panel"
            if self._selectedItemIndex == index then
                backgroundBoxSpriteName = "fantasy_9Panel_selected"
            end
            table.insert(self.widgetChildren, 
            {
                type = "backgroundbox",
                sprite = backgroundBoxSpriteName,
                scaleX="1.2,",
                scaleY="1.2",
                paddingBottom="32",
                paddingLeft="5",
                paddingRight="5",
                children = {
                    {
                        type = "static",
                        content = content,
                        paddingLeft = 5.0,
                        paddingTop = 10.0,
                        paddingRight = 5.0,
                        paddingBottom = 10.0,
                        scaleX="1.2",
                        scaleY="1.2",
                        sprite = spriteName,
                        children = {}
                    }
                }
            }
            )
        end
    - draws a background box around its child, scales with 9 panel scaling to have bordered windows
- ```radioGroup``` / ```radioButton```
    - use these two widgets to implement a radio button group
- ```slider```
    - slide it with the mouse
- ```static``` 
    - an image
    - (name from MFC class CStatic)
- ```textButton```
    - a clickable button with text in it
- ```textInput```
    - a text entry field
- ```text```
    - some text

For an exhaustive list of attributes of each widget type, read the source code.

Widgets are composable, that means widgets data objects and functions can be reused to implement more complex widgets. For example the Text Button widget reuses code and structs from the background box and Text widgets.
        return self.widgetChildren
    end,
```

In addition to the xml syntax for creating trees of widgets, they can be created from lua tables. The name of the element becomes a field called "type" in the table and a field called "children" lists the widgets children. Text content is set to the lua table field "content". DataNode.c/.h provide a wrapper that can either wrap a parsed xml tree or a lua table like this and widgets construct themselves from a DataNode.

When a bound property of any type has changed on the viewmodel side and it needs to inform the view, the lua function ```lua OnPropertyChanged(self, "PropertyName")``` needs to be called.

- Background Box
    - Bindable fields:
        - ```sprite```
- Radio Button / Radio Group
    - Bindable fields:
        - (radio group) ```selectedChild```
- Slider
    - Bindable fields:
        - ```val``` (two way)
- Static 
    - Bindable fields:
        - ```sprite```
- Text Button
    - Bindable fields:
- Text Entry
    - Bindable fields:
        - the xml nodes content
- Text Widget
    - Bindable fields:
        - the xml nodes content

In addition to bindings, UI events can be handled. I've kept this conceptually separate from bindings for now. All widget types can handle these mouse events:
- onMouseDown
- onMouseUp
- onMouseLeave
- onMouseEnter

they're used like so:

```xml
<backgroundboxo onMouseDown="OnNewGameMouseDown" onMouseLeave="OnNewGameMouseLeave" onMouseUp="OnNewGameMouseUp">
    ...
</backgroundbox>
```

These attributes all refer to to methods on the viewmodel object:

```lua
-- viewmodel...
OnNewGameMouseDown = function(self, x, y, button)
    if not self.newButtonPressed then
        self.newButtonPressed = true			
        OnPropertyChanged(self, "NewButtonBackgroundSprite")
    end
end,
OnNewGameMouseLeave = function (self, x, y) -- OnMouseEnter is the same args
    if self.newButtonPressed then
        self.newButtonPressed = false			
        OnPropertyChanged(self, "NewButtonBackgroundSprite")
    end
end,
OnNewGameMouseUp = function(self, x, y, button)
    if self.newButtonPressed then
        self.newButtonPressed = false			
        OnPropertyChanged(self, "NewButtonBackgroundSprite")
    end
end,
-- viewmodel...
```

Some widget types also have specific events they can handle:

- TextButton
    - ```onPress``` - callback when pressed (for) convenience

(there are probably others)

## Lua exposed functions

see engine/src/scripting/Scripting.c:

```c
	Sc_RegisterCFunction("OnPropertyChanged", &L_OnPropertyChanged);
	Sc_RegisterCFunction("SubscribeGameFrameworkEvent", &L_SubscribeToGameFrameworkEvent);
	Sc_RegisterCFunction("UnsubscribeGameFrameworkEvent", &L_UnSubscribeToGameFrameworkEvent);
	Sc_RegisterCFunction("FireGameFrameworkEvent", &L_FireGameFrameworkEvent);
	Sc_RegisterCFunction("PopGameFrameworkLayer", &L_PopGameFrameworkLayer);
	Sc_RegisterCFunction("GetButtonBinding", &L_GetButtonBinding);
	Sc_RegisterCFunction("FreeButtonBinding", &L_FreeButtonBinding);
	Sc_RegisterCFunction("GetButtonPress", &L_GetButtonPress);
	Sc_RegisterCFunction("GetGameLayerZoom", &L_GetGamelayerZoom);
	Sc_RegisterCFunction("SetGameLayerZoom", &L_SetGamelayerZoom);
	Sc_RegisterCFunction("CenterCameraAt", &L_CenterCameraAt);
```

## Text rendering

Freetype is used for text rendering, font sizes are rendered ahead of time at the time of atlas creation. Rendered fonts form part of the atlas, the same as sprites.

This is only half done... better documentation needed
