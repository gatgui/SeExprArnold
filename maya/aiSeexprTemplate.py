import maya.cmds as cmds
import maya.mel as mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEaiSeexprTemplate(ShaderAETemplate):
   
   def expressionChanged(self, nodeAttr, field):
      try:
         expr = cmds.scrollField(field, query=1, text=1)
         cmds.setAttr(nodeAttr, expr, type="string")
      except:
         pass
   
   def expressionUpdated(self, nodeAttr, field):
      try:
         cmds.scrollField(field, e=1, tx=cmds.getAttr(nodeAttr))
      except:
         pass
   
   def createExpression(self, nodeAttr):
      field = cmds.scrollField(ww=0)
      cmds.scrollField(field, e=1, cc=lambda *args: self.expressionChanged(nodeAttr, field))
      cmds.scrollField(field, e=1, tx=cmds.getAttr(nodeAttr))
      
      cmds.scriptJob(parent=field, attributeChange=(nodeAttr, lambda *args: self.expressionUpdated(nodeAttr, field)))
   
   def replaceExpression(self, nodeAttr):
      parent = cmds.setParent(query=1)
      children = cmds.layout(parent, query=1, childArray=1)
      
      if len(children) == 1:
         field = parent + "|" + children[0]
         
         cmds.scrollField(field, e=1, cc=lambda *args: self.expressionChanged(nodeAttr, field))
         cmds.scrollField(field, e=1, tx=cmds.getAttr(nodeAttr))
         
         cmds.scriptJob(parent=field, replacePrevious=1, attributeChange=(nodeAttr, lambda *args: self.expressionUpdated(nodeAttr, field)))
   
   def createFloatVariables(self, nodeAttr):
      # add/remove/moveup/movedown buttons
      # table: name -> value [floatFieldGrp]
      # col = cmds.columnLayout()
      # row = cmds.rowLayout(numberOfColumns=4)
      # cmds.button("Add")
      # cmds.button("Remove")
      # cmds.button("Up")
      # cmds.button("Down")
      # cmds.setParent("..")
      # cmds.setParent("..")
      pass
   
   def replaceFloatVariables(self, nodeAttr):
      pass
   
   def createVectortVariables(self, nodeAttr):
      pass
   
   def replaceVectortVariables(self, nodeAttr):
      pass
   
   def setup(self):
      self.addSwatch()
      
      self.beginScrollLayout()
      
      self.addCustom('expression', self.createExpression, self.replaceExpression)
      #self.addCustom('fparam_name', self.createFloatVariables, self.replaceFloatVariables)
      #self.suppress('fparam_value')
      #self.addCustom('vparam_name', self.createVectorVariables, self.replaceVectorVariables)
      #self.suppress('vparam_value')
      self.addControl('fparam_name')
      self.addControl('fparam_value')
      self.addControl('vparam_name')
      self.addControl('vparam_value')
      
      self.addControl('stop_on_error', label="Stop On Error")
      self.addControl('error_value', label="Error Value")
      
      mel.eval('AEdependNodeTemplate("%s")' % self.nodeName)
      self.addExtraControls()
      self.endScrollLayout()
