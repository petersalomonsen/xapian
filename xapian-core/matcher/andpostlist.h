/* andpostlist.h: Return only items which are in both sublists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_ANDPOSTLIST_H
#define OM_HGUARD_ANDPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"
#include "omdebug.h"

/** A postlist comprising two postlists ANDed together.
 *
 *  This postlist returns a posting if and only if it is in both of the
 *  sub-postlists.  The weight for a posting is the sum of the weights of
 *  the sub-postings.
 */
class AndPostList : public BranchPostList {
    protected:
        om_docid head;
        om_weight lmax, rmax;
    private:
	om_doccount dbsize;

        void process_next_or_skip_to(om_weight w_min, PostList *ret);
    public:
	om_doccount get_termfreq_max() const;
	om_doccount get_termfreq_min() const;
	om_doccount get_termfreq_est() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 *
	 *  The doclength returned by each branch should be the same.
	 *  The default implementation is simply to return the result
	 *  returned by the left branch: the left branch is preferable
	 *  because this should be the fastest way to get to a leaf node.
	 */
	virtual om_doclength get_doclength() const;

        AndPostList(PostList *left,
		    PostList *right,
		    MultiMatch *matcher_,
		    om_doccount dbsize_,
		    bool replacement = false);
};

inline om_doccount
AndPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, om_doccount, "AndPostList::get_termfreq_max", "");
    RETURN(std::min(l->get_termfreq_max(), r->get_termfreq_max()));
}

inline om_doccount
AndPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, om_doccount, "AndPostList::get_termfreq_min", "");
    RETURN(0u);
}

inline om_doccount
AndPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, om_doccount, "AndPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l and r) = P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    RETURN(static_cast<om_doccount> (lest * rest / dbsize));
}

inline om_docid
AndPostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "AndPostList::get_docid", "");
    RETURN(head);
}

// only called if we are doing a probabilistic AND
inline om_weight
AndPostList::get_weight() const
{
    DEBUGCALL(MATCH, om_weight, "AndPostList::get_weight", "");
    RETURN(l->get_weight() + r->get_weight());
}

// only called if we are doing a probabilistic operation
inline om_weight
AndPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, om_weight, "AndPostList::get_maxweight", "");
    RETURN(lmax + rmax);
}

inline om_weight
AndPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, om_weight, "AndPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    RETURN(AndPostList::get_maxweight());
}

inline bool
AndPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "AndPostList::at_end", "");
    RETURN(head == 0);
}

inline std::string
AndPostList::get_description() const
{
    return "(" + l->get_description() + " And " + r->get_description() + ")";
}

inline om_doclength
AndPostList::get_doclength() const
{
    DEBUGCALL(MATCH, om_doclength, "AndPostList::get_doclength", "");
    om_doclength doclength = l->get_doclength();
    DEBUGLINE(MATCH, "docid=" << head);
    AssertEqDouble(l->get_doclength(), r->get_doclength());
    RETURN(doclength);
}

#endif /* OM_HGUARD_ANDPOSTLIST_H */
