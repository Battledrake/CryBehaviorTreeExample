A small project to show how to code up a behavior tree

After project is downloaded, right click on Game.cryproject and generate solution. In order to make use of the Decorator Node(and possibly others with children) inside the behavior tree editor, the project has to be run in profile. A crash occurs when setting the child through the editor in Debug mode. If behavior tree needs to be debugged, create the tree in profile mode, then run in debug mode. It is only setting the child node through the editor in debug mode that causes the crash.

SideNote: All behavior trees must be saved to the scripts>AI>BehaviorTrees folder.
