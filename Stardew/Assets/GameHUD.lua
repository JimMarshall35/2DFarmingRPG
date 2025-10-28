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

function GetGameHUDViewModel()
    return {
        _inventoryChangedListener = nil,
        _items = { },--0, 1, 2, 3, 4, 5, 6, -1, -1, -1, -1, -1 },
        widgetChildren = {},
        InventoryChildren = function(self)
            self.widgetChildren = {}
            for index, item in pairs(self._items) do
                local spriteName = "no-item"
                if item.item >= 0 then
                    spriteName = WfGetItemSpriteName(item.item)
                end
                table.insert(self.widgetChildren, 
                {
                    type = "backgroundbox",
                    sprite="fantasy_9Panel",
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
            return self.widgetChildren
        end,
        OnInventoryChanged = function(self, msg)
            --print(dump(msg))
			self._items = msg
			OnPropertyChanged(self, "InventoryChildren")
		end,

        OnXMLUILayerPush = function(self)
            print("ONPUSHXML")
            self._inventoryChangedListener = SubscribeGameFrameworkEvent("InventoryChanged", self, self.OnInventoryChanged)
            OnPropertyChanged(self, "InventoryChildren")
            FireGameFrameworkEvent({vm=self, type="basic"}, "onHUDLayerPushed")
        end
    }
end
