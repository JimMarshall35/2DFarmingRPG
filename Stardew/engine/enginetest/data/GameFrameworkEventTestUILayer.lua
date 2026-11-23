function GetTestViewModel()
	return {
		updateInventoryListener,
		OnUpdateDisplayedInventory = function(self, args)
			TestCallback(args)
		end,
		OnXMLUILayerPush = function(self)
			self.updateInventoryListener = SubscribeGameFrameworkEvent("UpdateDisplayedInventory", self, self.OnUpdateDisplayedInventory)
			FireGameFrameworkEvent({ 
				prop="val",
				intProp=42,
				boolProp=false,
				floatProp=32.5
			}, "OnGameHUDPush")
		end,
		OnXMLUILayerPop = function(self)
			UnsubscribeGameFrameworkEvent(self.updateInventoryListener)
		end
	}
end