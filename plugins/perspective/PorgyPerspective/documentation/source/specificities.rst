.. _porgy_in_details:

*******************
Porgy in details
*******************

In the following section, you will find information on the advanced functionalities of *Porgy*. We will consider from here that you have successfully achieved the :ref:`tutorial <rewriting_example>` presenting how to create rules and apply them. We will thus base all the explanations in the direct continuity.

.. _property_manager:

Property Manager
================

The **Property Manager** is available in the *Edit* menu. The widget will display all the properties existing in the current graph and give you the possibility to select any possible combination of those properties to use as matching criterion. 

.. figure :: _images/property_manager.png
    :align: center
    :width: 50%

For instance two portnodes with different names but with a similar number of ports will be considered as similar elements by default. If you wish those two elements to be distinguish from each other, you can select the property *viewLabel* (containing their respective names) in the **Property Manager**. From now on, the subgraph matching operation will consider the selected property as an additional condition for the matching to be successful. In the screenshot above, the *state* and *viewLabel* properties have been selected by double-clicking in the second column on the appropriate lines; doing so makes a combo-box appear, offering to set the value to *true* or *false*.

New properties can be created by clicking the **+** button and outdated ones can be deleted by selecting them and using the **-** button. Please remember that the visual properties (whose name begin with *view*) are crucial for the visualizations and thus will be restored to their default value if deleted.


------------------
Optional matching
------------------

Finer control over the selected properties can be achieve when editing each rule, allowing for instance to only check the value on specific elements. When editing a rule in the :ref:`rule_view`, you can use the *Get information* interactor to open a widget listing the value of the properties of an element. For instance, in the case of the rules created in the previous tutorial, we take a peek at the values for the properties of the middle *yellow* portnode in rule **3yellow-red**:

.. figure :: _images/get_information_property_isused.png
    :align: center
    :width: 40%

The selection of the *viewColor* property in the *Property Manager* generates a boolean property called *viewColor_isused*. When set to *true*, the corresponding property is checked for this element during the matching. When set to *false* however, the property is still generally checked for the matching but the current element becomes an exception. If we set the *viewColor_isused* property value to *false* for the middle *yellow* portnode and its ports, any chain of three elements with *yellow* portnodes at its extremities will match. Applying the modified **3yellow-red** rule to *G11* in the derivation tree generated during the tutorial illustrates this perfectly:

.. figure :: _images/trace_property_isused.png
    :align: center
    :width: 40%

*G19* is generated with the **3yellow-red** rule application on *G11*, a result impossible to obtain for the original rule as *G11* only contains 2 *yellow* portnodes. This advanced checking possibility allows further control on the transformations and very fine tuning over the operations we are able to perform.


-------------------------------
Copying values during rewriting
-------------------------------

The boolean property (*\*_isused*) created after selecting a matching property in the **Property Manager** can also be modified on the elements of the right-hand side. This can be used to specify which value to affect during the rewriting process. We consider the graph used in the tutorial, the rule **red-blue** and the matching property *viewColor* to illustrate the process.

* After having selected *viewColor* in the **Property Manager** as a matching condition, the boolean property *viewColor_isused* is created.
* We select both of the blue ports and update their value for *viewColor_isused* to false.

.. figure :: _images/get_information_property_isused_rhs.png
    :align: center
    :width: 40%

* After applying the changed **red-blue** rule, we can witness the difference between the normal result (in *G1*) and the modified version (in *G2*) as the ports colours have not changed according to the visual information given by the rule.

.. figure :: _images/trace_property_isused_rhs.png
    :align: center
    :width: 40%

When set to *true*, the *\*_isused* property indicates the value given by the element in the right-hand side will be the one rewritten. This is the default behaviour thus the ports rewritten are blue (like *G1*), as expressed visually by the corresponding elements in the right-hand side. However, when set to *false*, the value selected is the one obtained from the matching elements corresponding to the linked element of the left-hand side. As a result, the application of the modified rule transform the *red* portnode in a *blue* portnode with red ports.

A similar treatment can applied to the **blue-2yellow** rule:

* for both of the blue ports in the left-hand side, the *viewColor_isused* property is set to false (desactivate the colour matching for those elements)
* for both of the linked ports in the right-hand side, the *viewColor_isused* property is set to false (keep the original colour of the ports matching the rule)
* we obtain the following rewriting result where both ports keep the original colour of their matching correspondances.

.. figure :: _images/trace_property_isused_rhs2.png
    :align: center
    :width: 40%

This kind of transformation makes sense when several matching properties are used at the same time but are not relevant in every rule. Specifying the local matching properties and whether the value must be passed to the rewritten element allows to rewrite and transform the graph in different ways. Please note that this behaviour is only available for ports as they are the sole elements in the right-hand side to be linked to other elements in the left-hand side.


The *Ban* property
==================

We have mentioned the *Position* property in the previous tutorial however a second property, the *Ban* property, can be used to limit the elements available for the rule application. 

Basically, an element can be in the *Position* set, in the *Ban* set or neither of those (neutral). When looking for elements during the matching of the LHS for each rule, at least one of the elements of the left-hand side has to be within the *Position* set for the matching to be valid. The matching will be impossible if all of the elements are *neutral*.

The *Ban* set is the exact opposite of the *Position* set, as such the elements *banned* will be plainly ignored during the matching step. The elements in the right-hand side of a rule can be send to the *Ban* set after the rewriting operation by setting the **N** property of each of those elements to *true*. 

Please note that the *Ban* property takes priority over the *Position* property.


Checking a rule application on a graph
======================================

The application of a rule is made by a drag and drop of a rule to:

* a graph metanode in the derivation tree view. If the operation is valid a green square will appear.
* a graph view.

Once you drop the rule a parameter configuration widget will appear to configure the rule application.

.. image :: _images/simple_rule_apply.png
    :align: center
    :width: 40%

You can customize these parameters:

* **Rule Name**
* **Maximum number of instances to apply**: the maximum number of matching to compute..
* **Position**: the boolean property that defines a subgraph of the graph. This subgraph is used to restrict the search of the instances of the left-hand side of the rule. When one tries to find the instances *I* in *G*, at least one node of each *I* must be in the *Position* set. It is possible to define more than one set *Position*.
* **Ban**: similar to *Position* but with the *Ban* property.

If the process works, the resulting graph will appear in the derivation tree, else a red state will appear.


Strategies
==========

.. _strategy_grammar:

A strategy is a small script language created to steer rule rewriting. See the website http://porgy.labri.fr/#download
for a complete documentation of the whole strategy language.
