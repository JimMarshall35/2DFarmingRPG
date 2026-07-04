
function UI_EncodeColour(r, g, b, a)
    -- colour values between 0 and 255
    return r .. "," .. g .. "," .. b .. "," .. a
end


function GetTextWidget(content, colour, padding)
    return {
        type = "text",
        content = content,
        paddingLeft = padding.left and tostring(padding.left) or nil,
        paddingTop = padding.top and tostring(padding.top) or nil,
        paddingRight = padding.right and tostring(padding.right) or nil,
        paddingBottom = padding.bottom and tostring(padding.bottom) or nil,
        colour = UI_EncodeColour(
            colour.r and colour.r or 0.0,
            colour.g and colour.g or 0.0, 
            colour.b and colour.b or 0.0, 
            colour.a and colour.a or 1.0
        ),
        children = {}
    }
end

function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

function GetTestViewModel()
	return {
        textList = {},
        counter = 0,
        entryLine = "Type here",

        Get_entryLine = function(self)
            print(self.entryLine)
            print("Get entryLine called")
            return self.entryLine
        end,

        Set_entryLine = function(self, val)
            self.entryLine = val
            print(self.entryLine)
        end,

        onClearPress = function(self)
            self.textList = {}
            OnPropertyChanged(self, "stackpanelChildren")
        end,

        stackpanelChildren = function(self)
            return self.textList
        end,

        onTextLineEnter = function(self)
            print("onTextLineEnter\n")
            tw = GetTextWidget(
                self.entryLine,
                { r=0.0, g=0.0, b=0.0 },
                { paddingTop=10.0 }
            )

            print(dump(tw))
            table.insert(self.textList, {
                type = "text",
                content = self.entryLine,
                paddingLeft = "20.0",
                paddingTop = "20.0",
                colour = math.random(255) .. "," .. math.random(255) .. "," .. math.random(255) ..",255",
                horizontalAlignment = "left",
                children = {}
            })
            OnPropertyChanged(self, "stackpanelChildren")
            self.entryLine = ""
            OnPropertyChanged(self, "entryLine")
        end

	}
end