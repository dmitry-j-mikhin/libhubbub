-- Post-receive hook for NetSurf repo, let's inform CIA of what's happening
local repo, updates = ...

empty = "0000000000000000000000000000000000000000"

for ref, data in pairs(updates) do
   if ref:sub(1,11) == "refs/heads/" and data.oldsha ~= empty and data.newsha ~= empty then
      log.chat("Informing CIA of commits on", ref:sub(12))
      cia.inform_commits("libhubbub", ref:sub(12), repo,
			 data.oldsha, data.newsha)
   else
      log.chat("Not informing CIA about", ref)
   end
end

