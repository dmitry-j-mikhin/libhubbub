-- Post-receive hook for NetSurf repo, let's inform CIA of what's happening
local repo, updates = ...

empty = "0000000000000000000000000000000000000000"
--[[
for ref, data in pairs(updates) do
   if ref:sub(1,11) == "refs/heads/" and data.oldsha ~= empty and data.newsha ~= empty then
      log.chat("Informing CIA of commits on", ref:sub(12))
      cia.inform_commits("libhubbub", ref:sub(12), repo,
			 data.oldsha, data.newsha)
   else
      log.chat("Not informing CIA about", ref)
   end
end
]]
if updates["refs/heads/master"] then
   log.state("Causing CI update, please wait...")
   local code, msg, headers, content = fetch("https://ci.netsurf-browser.org/jenkins/git/notifyCommit?url=git://git.netsurf-browser.org/libhubbub.git")
   if code ~= "200" then
      log.state("Did not get a 200!")
   end
   for line in content:gmatch("([^\r\n]*)\r?\n") do
      log.state("netsurf-ci: " .. line)
   end
end

