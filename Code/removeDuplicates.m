function out = removeDuplicates(a)
%Remove duplicate values from an array
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

out = [];
for i = 1:length(a)
    if isempty(find(out==a(i)))
        out(end+1) = a(i);
    end
end
