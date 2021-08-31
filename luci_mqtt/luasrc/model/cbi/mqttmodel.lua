require("luci.config")

local s, s2, m

m = Map("mqttconfig")

section = m:section(NamedSection, "mqttconfigsct", "mqttconfig", "Subscribe info")

flag = section:option(Flag, "enable", "Enable", "Enable program")
tls_ssl = section:option(Flag, "enabletsl", "TSL_SSL", "Enable program use TSL_SSL")

port = section:option( Value, "port", "Port")
port.datatype = "string"
port.placeholder = translate("1883 if using TSL/SSL use 8883 port")
port.rmempty = false


address = section:option( Value, "address", "Address")
address.datatype = [[ maxlength(100), "string"]]
address.rmempty = false



username = section:option(Value, "username","Username")
username.datatype = "credentials_validate"
username.placeholder = translate("Username")



password = section:option(Value, "password", translate"Password")
password.password = true
password.datatype = "credentials_validate"
password.placeholder = translate("Password")




---------------------------- Topic ----------------------------

st = m:section(TypedSection, "topic", translate("Topics"), translate("") )
st.addremove = true
st.anonymous = true
st.template = "cbi/tblsection"
st.novaluetext = translate("There are no topics created yet.")

topic = st:option(Value, "topic", translate("Topic name"), translate(""))
topic.datatype = "string"
topic.maxlength = 65536
topic.placeholder = translate("Topic")
topic.rmempty = false
topic.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg(translate("Topic name can not be empty"))
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end

--------------------- Program supplement  ---------------------
------------ With key, type, comparison type, value------------

s = m:section(TypedSection, "case", translate("Case"), translate("Add case on which would be mail sended"))
s.anonymous = true
s.addremove = true 
s.delete_alert = true
s.template = "cbi/tblsection"

event_topic = s:option(ListValue, "topics", translate("Topic"))
event_topic.rmempty = true
event_topic.multiple = true

m.uci:foreach("mqttconfig", "topic", function(section)

	if section.topic then
		event_topic:value(section.topic)
		exist = true
	end
end)


key = s:option(Value, "key", translate"Key")
key.datatype = [[ maxlength(100), "string"]]
key.rmempty = false

type = s:option(ListValue, "type", translate"Type")
type:value("0", "String");
type:value("1", "Decimal");
type.rmempty = false

comparisonType = s:option(ListValue, "comparisonType", translate"Comparison type")
comparisonType:value("!=", "!= - not equal");
comparisonType:value("==", "== - equal");
comparisonType:value("<=", "<= - less than or equal to");
comparisonType:value(">=", ">= -  more than or equal to");
comparisonType:value("<", "< - less");
comparisonType:value(">", "> - more");
comparisonType.rmempty = false

value = s:option(Value, "value", translate"Value")
value.datatype = [[ maxlength(100), "string"]]
value.rmempty = false

return m