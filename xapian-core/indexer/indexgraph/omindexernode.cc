/* omindexernode.cc: base class for the indexer network node.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "omindexernode.h"
#include "om/omerror.h"
#include <typeinfo>

Message
OmIndexerNode::get_output_record(const std::string &output_name)
{
    calculate_if_needed(output_name);
    std::map<std::string, Record>::iterator i;
    i = outputs_record.find(output_name);

    Message result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	// FIXME: first check outputs_type then recalculate.
	throw std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name();
    } else {
	auto_ptr<Record> temp(new Record(i->second));
	result = temp;
	outputs_record.erase(i);
    }
    return result;
}

void
OmIndexerNode::calculate_if_needed(const std::string &output_name)
{
    // FIXME: check non-record types too
    std::map<std::string, Record>::iterator i;
    i = outputs_record.find(output_name);

    if (i == outputs_record.end()) {
	calculate();
    }
}

OmIndexerNode::OmIndexerNode()
{
}


OmIndexerNode::~OmIndexerNode()
{
}

void
OmIndexerNode::config_modified(const std::string &key)
{
}

Record::Record() : type(mt_int)
{
    u.int_val = 0;
}

Record::Record(const Record &other)
	: type(other.type)
{
    switch (type) {
	case mt_int:
	    u.int_val = other.u.int_val;
	    break;
	case mt_double:
	    u.double_val = other.u.double_val;
	    break;
	case mt_string:
	    u.string_val = new string(*other.u.string_val);
	    break;
	case mt_vector:
	    u.vector_val = new vector<Record>(*other.u.vector_val);
	    break;
    }
}

void
Record::operator=(const Record &other)
{
    Record temp(other);
    swap(temp);
}

void
Record::swap(Record &other) {
    union {
	int int_val;
	double double_val;
	std::string *string_val;
	std::vector<Record> *vector_val;
    } tempu;
    switch (other.type) {
	case mt_int:
	    tempu.int_val = other.u.int_val;
	    break;
	case mt_double:
	    tempu.double_val = other.u.double_val;
	    break;
	case mt_string:
	    tempu.string_val = other.u.string_val;
	    break;
	case mt_vector:
	    tempu.vector_val = other.u.vector_val;
	    break;
    }
    /* now copy this union across */
    switch (type) {
	case mt_int:
	    other.u.int_val = u.int_val;
	    break;
	case mt_double:
	    other.u.double_val = u.double_val;
	    break;
	case mt_string:
	    other.u.string_val = u.string_val;
	    break;
	case mt_vector:
	    other.u.vector_val = u.vector_val;
	    break;
    }
    /* And now copy the temp union over ours */
    switch (other.type) {
	case mt_int:
	    u.int_val = tempu.int_val;
	    break;
	case mt_double:
	    u.double_val = tempu.double_val;
	    break;
	case mt_string:
	    u.string_val = tempu.string_val;
	    break;
	case mt_vector:
	    u.vector_val = tempu.vector_val;
	    break;
    }
    /* finally swap type */
    std::swap(type, other.type);
}

Record::~Record()
{
    switch (type) {
	case mt_int:
	case mt_double:
	    // nothing to be done
	    break;
	case mt_string:
	    delete u.string_val;
	    break;
	case mt_vector:
	    delete u.vector_val;
	    break;
    }
}

void
OmIndexerNode::connect_input(const std::string &input_name,
			     OmIndexerNode *other_node,
			     const std::string &other_outputname)
{
    input_desc con;
    con.node = other_node;
    con.node_output = other_outputname;

    inputs[input_name] = con;
}

void OmIndexerNode::set_output_record(const std::string &output_name,
				      const Record &value)
{
    // TODO: check that it isn't already set?
    outputs_record[output_name] = value;
}

Message OmIndexerNode::get_input_record(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     input_name);
    } else {
	return i->second.node->get_output_record(i->second.node_output);
    }
}

#if 0
Message
OmIndexerNode::get_input(std::string input_name)
{
    std::map<std::string, input_connection>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw (std::string("Request for input ") + 
	       input_name + " which isn't connected.");
    }

    return (i->second.node)->get_output(i->second.output_name);
}

OmOrigNode::OmOrigNode(Message message_)
	: message(message_)
{
    add_output("out", &OmOrigNode::get_out);
}

Message
OmOrigNode::get_out()
{
    return message;
}
#endif
