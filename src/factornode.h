#ifndef FACTORNODE_H
#define FACTORNODE_H


#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cstdlib>

class Network;

#include "message.h"




/**
 * @brief The FactorNode class basic abstract node
 */
class FactorNode
{
public:
    /**
     * @brief FactorNode
     * @param nwk the network containing this node
     */
    FactorNode(Network *nwk);

    virtual ~FactorNode() {}

    //! id accessor
    inline int id() const { return m_id; }

    //! propagate after receiving a message (i.e. receive in non-loopy mode)
    virtual void propagate(int from, const GaussianMessage &msg);

    //! receive update
    virtual void receive(int from, const GaussianMessage &msg);

    //! send update to a node (loopy mode)
    virtual void send(int to);

    //! add ingoing connection to the node
    virtual void addIncoming(FactorNode *node);

    //! add outgoing connection to the node
    virtual void addOutgoing(FactorNode *node);

    //! add a custom connection with the given tag
    virtual void addConnection(FactorNode *node, const std::string &tag);

    //! check if the message type is supported
    virtual bool isSupported(Message::Type type) = 0;

    //! get all the messages
    inline const MessageBox &messages() const
    {
        return m_messages;
    }

    //! get the message for the specified id
    inline const GaussianMessage &message(int node_id) const
    {
        MessageBox::const_iterator it = m_messages.find(node_id);
        if (it == m_messages.end())
            throw std::runtime_error("FactorNode(" + id_to_string(m_id) + ")::message: no message from node:" + id_to_string(node_id));
        return it->second;
    }

    //! add an incoming message from the node with the specified id
    inline void setMessage(int from, const GaussianMessage &msg)
    {
        if (!isSupported(msg.type()))
            throw std::runtime_error("FactorNode(" + id_to_string(m_id) + ")::addMessage: unsupported message type");
        std::pair<MessageBox::iterator, bool> res = m_messages.insert(std::make_pair(from, msg));
        if (!res.second)
            res.first->second = msg;
    }


protected:
    //! the action that this node is actually doing
    virtual GaussianMessage function(int to, const MessageBox &msgs) = 0;

    //! check if the message is forward or not
    inline bool isForward(int node_id) const
    {
        return m_outgoing.count(node_id) != 0;
    }

    inline bool isBackward(int node_id) const
    {
        return m_incoming.count(node_id) != 0;
    }

    /**
     * @brief isConnection check whether the node is a connection with the specified tag
     */
    inline bool isConnection(int node_id, const std::string &tag)
    {
        std::map<int, std::string>::iterator it = m_connections.find(node_id);
        return it == m_connections.end() ? false : it->second == tag;
    }

    //! check if there is a message for the given id
    inline bool hasMessage(int node_id) const
    {
        return m_messages.count(node_id) != 0;
    }


    //! the list of all nodes
    std::map<int, FactorNode*> m_nodes;

    //! incoming connections
    std::set<int> m_incoming;

    //! outgoing connections
    std::set<int> m_outgoing;

    //! custom connections
    std::map<int, std::string> m_connections;

private:
    //! the unique (TODO) id
    int m_id;

    //! all incoming messages
    MessageBox m_messages;

};






#endif // FACTORNODE_H
