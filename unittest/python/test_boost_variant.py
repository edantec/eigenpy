from boost_variant import V1, V2, VariantHolder, make_variant

variant = make_variant()
assert isinstance(variant, V1)

v1 = V1()
v1.v = 10

v2 = V2()
v2.v = "c"

variant_holder = VariantHolder()

# Test copy from variant alternative V1 to non initialized variant
variant_holder.variant = v1
assert isinstance(variant_holder.variant, V1)
assert variant_holder.variant.v == v1.v

# variant_holder.variant is a copy of v1
variant_holder.variant.v = 11
assert v1.v != variant_holder.variant.v

# Test variant_holder.variant return by reference
# v1 reference variant_holder.variant
v1 = variant_holder.variant
variant_holder.variant.v = 100
assert variant_holder.variant.v == 100
assert v1.v == 100
v1.v = 1000
assert variant_holder.variant.v == 1000
assert v1.v == 1000

# Test with the second alternative type
variant_holder.variant = v2
assert isinstance(variant_holder.variant, V2)
assert variant_holder.variant.v == v2.v
