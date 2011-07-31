local j=0
function a(i)
    print('a', j, i)
    j=j+1
    print(_("Ba\"\'z\nam", 1, 2, 3))
end    

j=10

print(a, _G.a)

for i, j in {'a', 'b', 'c', 'd', 'e'} do
    print(j)
    a(1)
    print(_("Bazquk"))
end

_("Foobar");

TR("Baz\n\n\n");
