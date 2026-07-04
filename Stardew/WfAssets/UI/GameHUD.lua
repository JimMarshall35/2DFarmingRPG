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
        _selectedItemChangedListener = nil,
        _items = { },--0, 1, 2, 3, 4, 5, 6, -1, -1, -1, -1, -1 },
        _selectedItemIndex = 1,
        widgetChildren = {},
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
                -- display the quantity of the item in the bottom right 
                local itemQuantity = {}
                if item.quantity > 1 then
                    itemQuantity = 
                    {
                        {
                            type = "text",
                            content = tostring(item.quantity),
                            paddingLeft = "0.0",
                            paddingTop = "0.0",
                            colour = "255,255,255,255",
                            dockPoint = "bottomRight",
                            paddingBottom = 5,
                            paddingRight = 5,
                            children = {}
                        }
                    }
                end
                table.insert(self.widgetChildren, 
                {
                    type = "backgroundbox",
                    sprite = backgroundBoxSpriteName,
                    scaleX="1.2,",
                    scaleY="1.2",
                    paddingBottom="32",
                    paddingLeft = "6",
                    paddingRight = "6",
                    children = {
                        {
                            type = "static",
                            content = content,
                            paddingLeft = 7.0,
                            paddingTop = 7.0,
                            paddingRight = 7.0,
                            paddingBottom = 7.0,
                            scaleX="1.2",
                            scaleY="1.2",
                            sprite = spriteName,
                            children = itemQuantity
                        }
                    }
                }
                )
            end
            return self.widgetChildren
        end,
        OnInventoryChanged = function(self, msg)
			self._items = msg
			OnPropertyChanged(self, "InventoryChildren")
		end,

        OnSelectedItemChanged = function(self, newIndexZeroBased)
            self._selectedItemIndex = newIndexZeroBased + 1
            OnPropertyChanged(self, "InventoryChildren")
        end,

        OnXMLUILayerPush = function(self)
            self._inventoryChangedListener = SubscribeGameFrameworkEvent("InventoryChanged", self, self.OnInventoryChanged)
            self._selectedItemChangedListener = SubscribeGameFrameworkEvent("SelectedItemChanged", self, self.OnSelectedItemChanged)
            OnPropertyChanged(self, "InventoryChildren")
            FireGameFrameworkEvent({vm=self, type="basic"}, "onHUDLayerPushed")
        end,

        OnXMLUILayerPop = function(self)
			UnsubscribeGameFrameworkEvent(self._inventoryChangedListener)
            UnsubscribeGameFrameworkEvent(self._selectedItemChangedListener)
		end

    }
end
