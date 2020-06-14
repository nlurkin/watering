/*
 * PublicationBase.h
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#ifndef PUBLICATIONBASE_H_
#define PUBLICATIONBASE_H_

#include <Arduino.h>
/*
 *
 */
class PublicationBase {
public:
	static constexpr size_t MAX_NAME_LENGTH=20;

	PublicationBase(const char *name);
	virtual ~PublicationBase();

	const char* getName() const {
		return _name;
	}

	bool isUpdated() const {
		return _updated;
	}

	void updated(bool val=true) {
		_updated = val;
	}

	virtual void to_string(char* buff) = 0;
	void to_string_base(char** buff);

private:
	bool _updated;
	char _name[MAX_NAME_LENGTH+1];
};

#endif /* PUBLICATIONBASE_H_ */
