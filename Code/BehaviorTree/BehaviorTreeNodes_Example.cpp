//Gotta have that header!
#include "BehaviorTreeNodes_Example.h"

//Needed for IBehaviorTreeManager
#include <CryAISystem/BehaviorTree/IBehaviorTree.h>

//Needed to inherit from Node
#include <CryAISystem/BehaviorTree/Node.h>

//We are receiving our event with a decorator, need this for that!
#include <CryAISystem/BehaviorTree/Decorator.h>

//Needed for the REGISTER macro. Macro defined in BehaviorTreeDefines.h
#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
#include <CrySerialization/Enum.h>
#include <CrySerialization/ClassFactory.h>
#endif

//Needed to access classes like Node and IBehaviorTreeManager(in addition to includes)
using namespace BehaviorTree;

//We inherit from Node class for basic behavior tree nodes. Can also extend from Decorator and Composite if that style of node is needed.
class ExampleLog : public Node
{
	//Appears to be standard in CryEngine source to create a typedef of NodeName to BaseClass. Used to reference the parent when needed.
	typedef Node BaseClass;

public:
	//Create this struct with every node. Can pass in node specific member variables to be used throughout node lifetime(NOT blackboard variables).
	//Example: a float timer variable that resets to 0 everytime the node is initialized, and is used to check how much time has passed in the update.
	struct RuntimeData
	{
		RuntimeData()
		{
		}
	};

	//Called when the xml is loaded from the behavior tree editor.
	virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const struct LoadContext& context, const bool isLoadingFromEditor) override
	{
		m_logMessage = xml->getAttr("message"); //retrieve any variables saved to xml, here we load the variable using the key "message"

		return LoadSuccess;
	}

	//Called when save is selected from the behavior tree editor. Macro defined from BehaviorTreeDefines.h during non release builds.
#ifdef USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
	virtual XmlNodeRef CreateXmlDescription() override
	{
		XmlNodeRef xml = BaseClass::CreateXmlDescription();
		xml->setTag("ExampleLog"); //Must be the same as the NodeName. NodeCreator retrieves this tag to create the node when xml is loaded.
		xml->setAttr("message", m_logMessage); //Set any variables we want using a key/value. Value is retrieved at load using key.
		return xml;
	}
#endif

	//Serializes any variables to modify inside the behavior tree editor. Macro is defined during non release builds in BehaviorTreeDefines.h
#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
	virtual void Serialize(Serialization::IArchive& archive) override
	{
		archive(m_logMessage, "message", "^Message"); //Here we serialize our variable to modify within the editor

		BaseClass::Serialize(archive);
	}
#endif

protected:
	//Called right before Update when Node is started. Called everytime the node is restarted.
	//A good place to set RuntimeData base values and other initialization logic.
	virtual void OnInitialize(const UpdateContext& context) override
	{

	}
	//Called by the parent Node right after initialize. Runs like an update method as long as the return value is Running.
	//Parent determines what happens when node returns Failure/Success
	virtual Status Update(const UpdateContext& context) override
	{
		//context is used to retrieve behavior tree info. Can get entity& owner, behavior tree, blackboard, etc... 
		//Used to grab any needed components, set/get blackboard variables, etc...

		//When the update is called, a log message will appear, then return success, allowing the parent to move on to the next flow of logic
		CryLogAlways(m_logMessage);
		return Success;
	}

	//Called by the parent when the Node is done running. Set any termination logic needed here.
	virtual void OnTerminate(const UpdateContext& context) override
	{
	}

private:
	//Here you run any event based logic. HandleEvent is called by the BehaviorTreeManager when an event is sent out. You do not call HandleEvent directly.
	virtual void HandleEvent(const EventContext& context, const Event& event) override
	{
	}

	//Any variables wanted for serialization and xml saving/loading, not the same as RuntimeData variables.
	//We want to serialize the message the log will print so we can set it in the behavior tree editor.
	string m_logMessage;
};

//Here I will demonstrate how to send an event!
//Ideally, you'd likely send the event from outside a node from something inside the gameworld. But BT nodes can do so too!
class ExampleSendEvent : public Node
{
	typedef Node BaseClass;

public:
	struct RuntimeData
	{
		RuntimeData()
		{
		}
	};

	virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const struct LoadContext& context, const bool isLoadingFromEditor)
	{
		const char* eventName = xml->getAttr("event");
		m_eventToSend = Event(eventName); //Now we create the event using the string set in the editor.
		return LoadSuccess;
	}

#ifdef USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
	virtual XmlNodeRef CreateXmlDescription() override
	{
		XmlNodeRef xml = BaseClass::CreateXmlDescription();
		xml->setTag("ExampleSendEvent");
		xml->setAttr("event", m_eventToSend.GetName()); //We use string to reference event because event type can't be serialized
		return xml;
	}
#endif

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
	virtual void Serialize(Serialization::IArchive& archive) override
	{
		archive(m_eventToSend.GetName(), "event", "^Event"); //Again, we serialize the name not the actual event
		BaseClass::Serialize(archive);
	}
#endif

protected:
	virtual void OnInitialize(const UpdateContext& context) override
	{
	}
	virtual Status Update(const UpdateContext& context) override
	{
		//This is how we call the event! Can be called after logic, inside the other methods, or by other non bt nodes.
		gEnv->pAISystem->GetIBehaviorTreeManager()->HandleEvent(context.entityId, m_eventToSend);
		return Running;
	}

	virtual void OnTerminate(const UpdateContext& context) override
	{
	}

private:
	virtual void HandleEvent(const EventContext& context, const Event& event) override
	{
	}

	Event m_eventToSend; //We create a new event of type event.
};


//NOTE: Custom Decorators cause a crash in the behavior tree editor(building from xml is fine) when in debug.
#if defined(_PROFILE)
//Events are only received when a node is running, so states/decorators/sequence/selectors, that have children, are the go to choice to listen for events.
//Parallel works too, to keep checking both nodes(the sender and the receiver) simultaneously, but i've went with Decorator for this showcasing.
class ExampleReceiveEvent : public Decorator
{
	typedef Decorator BaseClass;

public:
	struct RuntimeData
	{
		RuntimeData()
		{
		}
	};

	virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const struct LoadContext& context, const bool isLoadingFromEditor) override
	{
		LoadResult result = BaseClass::LoadFromXml(xml, context, isLoadingFromEditor);
		const char* eventName = xml->getAttr("event");
		m_eventToReceive = Event(eventName);
		return result;
	}

#ifdef USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
	virtual XmlNodeRef CreateXmlDescription() override
	{
		XmlNodeRef xml = BaseClass::CreateXmlDescription();
		xml->setTag("ExampleReceiveEvent");
		xml->setAttr("event", m_eventToReceive.GetName()); //We use string to reference event because event type can't be serialized
		return xml;
	}
#endif

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
	virtual void Serialize(Serialization::IArchive& archive) override
	{
		archive(m_eventToReceive.GetName(), "event", "^Event"); //Again, we serialize the name not the actual event

		BaseClass::Serialize(archive);
	}
#endif

protected:

	virtual void OnInitialize(const UpdateContext& context) override
	{
		BaseClass::OnInitialize(context);
	}
	virtual Status Update(const UpdateContext& context) override
	{
		return BaseClass::Update(context);
	}

	virtual void OnTerminate(const UpdateContext& context) override
	{
		BaseClass::OnTerminate(context);
	}

private:
	virtual void HandleEvent(const EventContext& context, const Event& event) override
	{
		//Normally, parents send down the events they received from the behaviortreemanager.
		//We're commenting since the child will be sending the event.
		//No need to create a loop of event sending. 
 //   		BaseClass::HandleEvent(context, event);
		if (event == m_eventToReceive)
		{
			CryLogAlways("We got the event!");
		}
	}

	Event m_eventToReceive; //We create a new event of type event.
};
#endif

//Free function called from GamePlugin.
//I call it in the ESYSTEM_EVENT_GAME_POST_INIT: event(Has to be called after Initialization since it retrieves the environment AISystem and BehaviorTreeManager).
void RegisterExampleNodes()
{

	//We need to pass a reference to the Register, so we get a pointer first, confirm it's not null, then create our reference variable.
	IBehaviorTreeManager* pManager = gEnv->pAISystem->GetIBehaviorTreeManager();
	if (pManager)
	{
		IBehaviorTreeManager& manager = *pManager;

		//We register every node we created. Use the manager variable name, TypeName of the custom BT Node, label for BT editor, and a color.
		REGISTER_BEHAVIOR_TREE_NODE_WITH_SERIALIZATION(manager, ExampleLog, "Custom\\ExampleLog", "000000");
		REGISTER_BEHAVIOR_TREE_NODE_WITH_SERIALIZATION(manager, ExampleSendEvent, "Custom\\ExampleSendEvent", "000000");
#if defined(_PROFILE)
		REGISTER_BEHAVIOR_TREE_NODE_WITH_SERIALIZATION(manager, ExampleReceiveEvent, "Custom\\ExampleReceiveEvent", "000000");
#endif
	}
}
