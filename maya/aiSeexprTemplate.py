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
      #cmds.scrollField(field, e=1, cc=lambda *args: self.expressionChanged(nodeAttr, field))
      #cmds.scrollField(field, e=1, tx=cmds.getAttr(nodeAttr))
      #
      #cmds.scriptJob(parent=field, attributeChange=(nodeAttr, lambda *args: self.expressionUpdated(nodeAttr, field)))
      self.replaceExpression(nodeAttr)
   
   def replaceExpression(self, nodeAttr):
      parent = cmds.setParent(query=1)
      children = cmds.layout(parent, query=1, childArray=1)
      
      field = parent + "|" + children[0]
      
      cmds.scrollField(field, e=1, cc=lambda *args: self.expressionChanged(nodeAttr, field))
      cmds.scrollField(field, e=1, tx=cmds.getAttr(nodeAttr))
      
      cmds.scriptJob(parent=field, replacePrevious=1, attributeChange=(nodeAttr, lambda *args: self.expressionUpdated(nodeAttr, field)))
   
   def removeFloatVariable(self, nodeAttr, varslayout):
      pass
   
   def addFloatVariable(self, nodeAttr, varslayout):
      n = cmds.columnLayout(varslayout, query=1, numberOfChildren=1)
      
      form = cmds.formLayout(numberOfDivisions=100, parent=varslayout)
      
      rembtn = cmds.button(label="-")
      namefld = cmds.textField(text="")
      vallbl = cmds.text(label="=")
      valfld = cmds.floatField(value=0.0)
      
      cmds.formLayout(form, edit=1,
                      attachForm=[(rembtn, "top", 0), (rembtn, "bottom", 0), (rembtn, "left", 0),
                                  (namefld, "top", 0), (namefld, "bottom", 0),
                                  (vallbl, "top", 0), (vallbl, "bottom", 0),
                                  (valfld, "top", 0), (valfld, "bottom", 0)],
                      attachControl=[(namefld, "left", 5, rembtn),
                                     (vallbl, "left", 5, namefld),
                                     (valfld, "left", 5, vallbl)],
                      attachNone=[(rembtn, "right"),
                                  (vallbl, "right")],
                      attachPosition=[(namefld, "right", 0, 30),
                                      (valfld, "right", 0, 100)])
   
   def removeVectorVariable(self, nodeAttr, varslayout):
      pass
   
   def addVectorVariable(self, nodeAttr, varslayout):
      n = cmds.columnLayout(varslayout, query=1, numberOfChildren=1)
      
      form = cmds.formLayout(numberOfDivisions=100, parent=varslayout)
      
      rembtn = cmds.button(label="-")
      namefld = cmds.textField(text="")
      vallbl = cmds.text(label="=")
      val0fld = cmds.floatField(value=0.0)
      val1fld = cmds.floatField(value=0.0)
      val2fld = cmds.floatField(value=0.0)
      
      cmds.formLayout(form, edit=1,
                      attachForm=[(rembtn, "top", 0), (rembtn, "bottom", 0), (rembtn, "left", 0),
                                  (namefld, "top", 0), (namefld, "bottom", 0),
                                  (vallbl, "top", 0), (vallbl, "bottom", 0),
                                  (val0fld, "top", 0), (val0fld, "bottom", 0),
                                  (val1fld, "top", 0), (val1fld, "bottom", 0),
                                  (val2fld, "top", 0), (val2fld, "bottom", 0)],
                      attachControl=[(namefld, "left", 5, rembtn),
                                     (vallbl, "left", 5, namefld),
                                     (val0fld, "left", 5, vallbl),
                                     (val1fld, "left", 5, val0fld),
                                     (val2fld, "left", 5, val1fld)],
                      attachNone=[(rembtn, "right"),
                                  (vallbl, "right")],
                      attachPosition=[(namefld, "right", 0, 30),
                                      (val0fld, "right", 0, 53),
                                      (val1fld, "right", 0, 76),
                                      (val2fld, "right", 0, 100)])
   
   def removeAllVariables(self, nodeAttr, varslayout):
      children = cmds.columnLayout(varslayout, query=1, childArray=1)
      for child in children:
         cmds.deleteUI(child)
   
   def createFloatVariables(self, nodeAttr):
      form = cmds.formLayout(numberOfDivisions=100)
      
      btnrow = cmds.rowLayout(numberOfColumns=2)
      addbtn = cmds.button(label="Add New")
      rembtn = cmds.button(label="Remove All")
      cmds.setParent("..")
      
      varslayout = cmds.columnLayout(rowSpacing=5, adjustableColumn=True, columnAttach=("both", 5))
      cmds.setParent("..")
      
      cmds.formLayout(form, edit=1,
                      attachForm=[(btnrow, "top", 5),
                                  (btnrow, "left", 5),
                                  (btnrow, "right", 5),
                                  (varslayout, "left", 5),
                                  (varslayout, "right", 5)],
                      attachControl=[(varslayout, "top", 5, btnrow)],
                      attachNone=[(btnrow, "bottom"),
                                  (varslayout, "bottom")])
      
      cmds.setParent("..")
      
      self.replaceFloatVariables(nodeAttr)
   
   def replaceFloatVariables(self, nodeAttr):
      parent = cmds.setParent(query=1)
      children = cmds.layout(parent, query=1, childArray=1)
      
      form = parent + "|" + children[0]
      children = cmds.layout(form, query=1, childArray=1)
      
      rowlayout = form + "|" + children[0]
      varslayout = form + "|" + children[1]
      
      children = cmds.layout(rowlayout, query=1, childArray=1)
      
      addbtn = rowlayout + "|" + children[0]
      rembtn = rowlayout + "|" + children[1]
      
      cmds.button(addbtn, edit=1, command=lambda *args: self.addFloatVariable(nodeAttr, varslayout))
      cmds.button(rembtn, edit=1, command=lambda *args: self.removeAllVariables(nodeAttr, varslayout))
      
      # Update content
   
   def createVectorVariables(self, nodeAttr):
      form = cmds.formLayout(numberOfDivisions=100)
      
      btnrow = cmds.rowLayout(numberOfColumns=2)
      addbtn = cmds.button(label="Add New")
      rembtn = cmds.button(label="Remove All")
      cmds.setParent("..")
      
      varslayout = cmds.columnLayout(rowSpacing=5, adjustableColumn=True, columnAttach=("both", 5))
      cmds.setParent("..")
      
      cmds.formLayout(form, edit=1,
                      attachForm=[(btnrow, "top", 5),
                                  (btnrow, "left", 5),
                                  (btnrow, "right", 5),
                                  (varslayout, "left", 5),
                                  (varslayout, "right", 5)],
                      attachControl=[(varslayout, "top", 5, btnrow)],
                      attachNone=[(btnrow, "bottom"),
                                  (varslayout, "bottom")])
      
      cmds.setParent("..")
      
      self.replaceVectorVariables(nodeAttr)
   
   def replaceVectorVariables(self, nodeAttr):
      parent = cmds.setParent(query=1)
      children = cmds.layout(parent, query=1, childArray=1)
      
      form = parent + "|" + children[0]
      children = cmds.layout(form, query=1, childArray=1)
      
      rowlayout = form + "|" + children[0]
      varslayout = form + "|" + children[1]
      
      children = cmds.layout(rowlayout, query=1, childArray=1)
      
      addbtn = rowlayout + "|" + children[0]
      rembtn = rowlayout + "|" + children[1]
      
      cmds.button(addbtn, edit=1, command=lambda *args: self.addVectorVariable(nodeAttr, varslayout))
      cmds.button(rembtn, edit=1, command=lambda *args: self.removeAllVariables(nodeAttr, varslayout))
      
      # Update content
   
   def setup(self):
      self.addSwatch()
      
      self.beginScrollLayout()
      
      self.addCustom('expression', self.createExpression, self.replaceExpression)
      
      self.beginLayout("Float variables", collapse=False)
      self.addCustom('fparam_name', self.createFloatVariables, self.replaceFloatVariables)
      self.suppress('fparam_value')
      self.endLayout()
      
      self.beginLayout("Vector variables", collapse=False)
      self.addCustom('vparam_name', self.createVectorVariables, self.replaceVectorVariables)
      self.suppress('vparam_value')
      self.endLayout()
      
      self.addControl('stop_on_error', label="Stop On Error")
      self.addControl('error_value', label="Error Value")
      
      mel.eval('AEdependNodeTemplate("%s")' % self.nodeName)
      self.addExtraControls()
      self.endScrollLayout()
