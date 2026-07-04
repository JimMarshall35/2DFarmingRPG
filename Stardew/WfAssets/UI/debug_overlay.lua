function GetDebugOverlayViewModel()
	return {
		_debugString = "debug message goes here",
		_debugStringListener = nil,
		OnDebugMessagePublished = function(self, msg)
			self._debugString = msg
			OnPropertyChanged(self, "DebugString")
		end,
		Get_DebugString = function(self)
			return self._debugString
		end,
		OnXMLUILayerPush = function(self)
			self._debugStringListener = SubscribeGameFrameworkEvent("DebugMessage", self, self.OnDebugMessagePublished)
			FireGameFrameworkEvent({vm=self, type="basic"}, "onDebugLayerPushed")
		end
	}
end