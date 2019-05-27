-- Thanks to prokopiu from X-Plane.org

-- 05-12-2019

dataref("TEXTOUT", "sim/operation/prefs/text_out", "writeable")

function speakNoText(sText)
  TEXTOUT=false
  XPLMSpeakString(sText)
  TEXTOUT=true
end

speakNoText("Welcome Captain") 
