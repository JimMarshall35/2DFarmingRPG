
function GetTestViewModel()
	return {
		-- idea

		Get_ListBoxItems = function(self)
			return {
				
			}
		end,
		
		-- random data members
		newButtonPressed = false,
		difficulty = 1,
		sliderVal = 0.0,

		-- two way binding
		Get_SliderVal = function(self)
			return self.sliderVal
		end,
		Set_SliderVal = function(self, val)
			--print(val)
			self.sliderVal = val
		end,

		-- a one way binding property called NewButtonBackgroundSprite
		Get_NewButtonBackgroundSprite = function(self)
			if self.newButtonPressed then 
				return "defaultPressedButton" 
			else 
				return "defaultButton"
			end
		end,

		-- a two way binding
		Get_Difficulty = function(self)
			return self.difficulty
		end,
		Set_Difficulty = function(self, difficulty)
			print("difficulty changed", difficulty)
			self.difficulty = difficulty
		end,

		-- event handlers that can be bound to individual widgets
		OnNewGameMouseDown = function(self, x, y, button)
			if not self.newButtonPressed then
				self.newButtonPressed = true			
				OnPropertyChanged(self, "NewButtonBackgroundSprite")
			end
		end,
		OnNewGameMouseLeave = function (self, x, y)
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

		OnLoadPress = function(self)
			print("LOAD PRESSED")
			self.difficulty = 0
			OnPropertyChanged(self,"Difficulty")
		end
	}
end