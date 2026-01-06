use crate::node::Node;

pub struct LinkedList<T> {
    head: Option<Box<Node<T>>>,
}

impl<T> LinkedList<T> {
    pub fn new() -> Self {
        LinkedList { head: None }
    }

    pub fn push(&mut self, val: T) {
        let new_node = Box::new(Node {
            val,
            next: self.head.take(),
        });
        self.head = Some(new_node);
    }

    pub fn pop(&mut self) -> Option<T> {
        self.head.take().map(|node| {
            self.head = node.next;
            node.val
        })
    }

    pub fn search(&self, target: &T) -> bool
    where
        T: PartialEq,
    {
        let mut current = &self.head;
        while let Some(node) = current {
            if &node.val == target {
                return true;
            }
            current = &node.next;
        }
        false
    }

    pub fn is_empty(&self) -> bool {
        self.head.is_none()
    }
}

impl<T> Drop for LinkedList<T> {
    fn drop(&mut self) {
        let mut current = self.head.take();
        while let Some(mut node) = current {
            current = node.next.take();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_push_and_pop() {
        let mut list = LinkedList::new();

        list.push(10);
        list.push(20);
        list.push(30);

        assert_eq!(list.pop(), Some(30));
        assert_eq!(list.pop(), Some(20));
        assert_eq!(list.pop(), Some(10));
        assert_eq!(list.pop(), None);
    }

    #[test]
    fn test_search() {
        let mut list = LinkedList::new();
        list.push(5);
        list.push(10);
        list.push(15);

        assert_eq!(list.search(&10), true);
        assert_eq!(list.search(&99), false);

        list.pop(); // Removes 15
        assert_eq!(list.search(&15), false);
    }

    #[test]
    fn test_empty_list() {
        let mut list: LinkedList<i32> = LinkedList::new();
        assert!(list.is_empty());
        assert_eq!(list.pop(), None);
        assert_eq!(list.search(&1), false);
    }

    #[test]
    fn test_strings() {
        let mut list = LinkedList::new();
        list.push("Hello".to_string());
        list.push("World".to_string());

        assert!(list.search(&"World".to_string()));
        assert_eq!(list.pop(), Some("World".to_string()));
    }
}
