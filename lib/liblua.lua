function ipairs(t)
    return function(s, _)
        s.idx = s.idx + 1
        if t[s.idx] then
            return s.idx, t[s.idx]
        else
            return nil, nil
        end
    end, { idx = 0 }, nil
end
