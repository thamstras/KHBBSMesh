# RUN WITH THE ARMATURE SELECTED IN EDIT MODE
import bpy

for bone in bpy.data.armatures["Armature"].edit_bones[:]:
    bone.inherit_scale = 'NONE'