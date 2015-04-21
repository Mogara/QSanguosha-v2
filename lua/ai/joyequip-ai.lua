sgs.ai_skill_invoke.grab_peach = function(self, data) 
    local from = data:toCardUse().from
    return self:isEnemy(from)
end
