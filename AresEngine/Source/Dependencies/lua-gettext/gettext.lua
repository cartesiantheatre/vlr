-- gettext.lua
-- $Id: gettext.lua,v 1.6 2007/10/30 21:55:22 cpressey Exp $
-- Lua wrapper functions for Lua 5.0.x gettext binding.

-- BEGIN gettext.lua --

GetText = require("lgettext")

function _(str, ...)
	return string.format(GetText.translate(str), ...)
end

return GetText

-- END of gettext.lua --
